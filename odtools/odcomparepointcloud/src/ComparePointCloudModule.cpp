/**
 * odComparePointCloudModule - Tool for comparing data between shared point cloud and 
 * compact point cloud
 * Copyright (C) 2017 Hang
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>

#include "opendavinci/odcore/base/CommandLineParser.h"
#include "opendavinci/odtools/player/Player.h"
#include "opendavinci/odcore/base/Lock.h"
#include "opendavinci/odcore/wrapper/SharedMemoryFactory.h"
#include "opendavinci/odcore/base/KeyValueConfiguration.h"

#include "ComparePointCloudModule.h"

namespace odcomparepointcloud {

    using namespace std;
    using namespace odcore::data;
    using namespace odtools::player;

    ComparePointCloudModule::ComparePointCloudModule(const int32_t &argc, char **argv) :
        TimeTriggeredConferenceClientModule(argc, argv, "ComparePointCloud"),
        m_CPCfound(false),
        m_SPCfound(false),
        m_frameNumber(0),
        m_spc(),
        m_cpc(),
        m_hasAttachedToSharedImageMemory(false),
        m_spcSharedMemory(NULL),
        m_xCpc(),
        m_yCpc(),
        m_zCpc(),
        m_xSpc(),
        m_ySpc(),
        m_zSpc(),
        m_xError(),
        m_yError(),
        m_zError(),
        m_outputData("output.csv", std::ios_base::app | std::ios_base::out),
        m_cpcFrame("cpcFrame.csv", std::ios_base::app | std::ios_base::out),
        m_spcFrame("spcFrame.csv", std::ios_base::app | std::ios_base::out),
        m_recordingFile(""),
        m_allFrames(false),
        m_chosenFrame(0),
        m_currentFrame(0) {
            //The vertical angles sorted by sensor IDs from 0 to 15 according to the data sheet
            m_verticalAngles[0] = -15.0;
            m_verticalAngles[1] = 1.0;
            m_verticalAngles[2] = -13.0;
            m_verticalAngles[3] = 3.0;
            m_verticalAngles[4] = -11.0;
            m_verticalAngles[5] = 5.0;
            m_verticalAngles[6] = -9.0;
            m_verticalAngles[7] = 7.0;
            m_verticalAngles[8] = -7.0;
            m_verticalAngles[9] = 9.0;
            m_verticalAngles[10] = -5.0;
            m_verticalAngles[11] = 11.0;
            m_verticalAngles[12] = -3.0;
            m_verticalAngles[13] = 13.0;
            m_verticalAngles[14] = -1.0;
            m_verticalAngles[15] = 15.0;
            
            //Distance values are reordered with increasing vertical angles while storing CPC
            m_sensorOrderIndex[0] = 0;
            m_sensorOrderIndex[1] = 2;
            m_sensorOrderIndex[2] = 4;
            m_sensorOrderIndex[3] = 6;
            m_sensorOrderIndex[4] = 8;
            m_sensorOrderIndex[5] = 10;
            m_sensorOrderIndex[6] = 12;
            m_sensorOrderIndex[7] = 14;
            m_sensorOrderIndex[8] = 1;
            m_sensorOrderIndex[9] = 3;
            m_sensorOrderIndex[10] = 5;
            m_sensorOrderIndex[11] = 7;
            m_sensorOrderIndex[12] = 9;
            m_sensorOrderIndex[13] = 11;
            m_sensorOrderIndex[14] = 13;
            m_sensorOrderIndex[15] = 15;
            
            for(uint8_t counter=0;counter<16;counter++){
                m_16SortedDistances[counter]=0.0;
            }
        }

    ComparePointCloudModule::~ComparePointCloudModule() {}
    
    void ComparePointCloudModule::setUp() {
        m_recordingFile = getKeyValueConfiguration().getValue<string>("ComparePointCloud.recording");
        m_allFrames = getKeyValueConfiguration().getValue< uint16_t >("ComparePointCloud.compareAllFrames") == 1;
        m_chosenFrame = getKeyValueConfiguration().getValue< uint64_t >("ComparePointCloud.frame");
    }

    void ComparePointCloudModule::tearDown() {}
    
    /*void ComparePointCloudModule::parseAdditionalCommandLineParameters(const int &argc, char **argv) {
        odcore::base::CommandLineParser cmdParser;
        cmdParser.addCommandLineArgument("frame");
        cmdParser.parse(argc, argv);
        odcore::base::CommandLineArgument cmdArgumentFRAME = cmdParser.getCommandLineArgument("frame");

        if (cmdArgumentFRAME.isSet()) {
            string frameOption = cmdArgumentFRAME.getValue<string>();
            if(frameOption=="all"){
                m_allFrames=true;
                cout<<"Compare all frames."<<endl;
            }
            else{
                m_chosenFrame = cmdArgumentFRAME.getValue<uint64_t>();
                cout<<"Compare Frame "<<m_chosenFrame<<endl;
            }
        }
        else{
            cerr<<"Please indicate a frame number to compare. Specify it with --frame."<<endl;
        }

    }*/
    
    void ComparePointCloudModule::readCPC(Container &c){
        m_cpc = c.getData<CompactPointCloud>();  
        float startAzimuth = m_cpc.getStartAzimuth();
        float endAzimuth = m_cpc.getEndAzimuth();
        uint8_t entriesPerAzimuth = m_cpc.getEntriesPerAzimuth();
        string distances = m_cpc.getDistances();
        uint32_t numberOfPoints = distances.size()/2;
        uint32_t numberOfAzimuths = numberOfPoints/entriesPerAzimuth;
        float azimuthIncrement = (endAzimuth-startAzimuth)/numberOfAzimuths;//Calculate the azimuth increment
        stringstream sstr(distances);
        const float toRadian = static_cast<float>(M_PI) / 180.0f;
        uint16_t distance_integer=0;
        float xyDistance = 0;
        float azimuth = startAzimuth;
        for (uint32_t azimuthIndex = 0; azimuthIndex < numberOfAzimuths; azimuthIndex++) {
            for (uint8_t sensorIndex = 0; sensorIndex<entriesPerAzimuth; sensorIndex++) {
                sstr.read((char*)(&m_16SortedDistances[m_sensorOrderIndex[sensorIndex]]), 2); // Read distance value from the string in a CPC container point by point
            }
            
            for (uint8_t sensorIndex = 0; sensorIndex<entriesPerAzimuth; sensorIndex++) {
                distance_integer=m_16SortedDistances[sensorIndex];
                if(distance_integer>=100){
                    float distance = static_cast<float>(distance_integer/100.0f); //convert cm to meter
                    // Compute x, y, z coordinate based on distance, azimuth, and vertical angle
                    xyDistance = distance * cos(m_verticalAngles[sensorIndex] * toRadian);
                    m_xCpc.push_back(xyDistance * sin(azimuth * toRadian));
                    m_yCpc.push_back(xyDistance * cos(azimuth * toRadian));
                    m_zCpc.push_back(distance * sin(m_verticalAngles[sensorIndex] * toRadian));
                }
            }
            azimuth+=azimuthIncrement;
        }
    }
    
    void ComparePointCloudModule::readSPC(Container &c){
        m_spc = c.getData<SharedPointCloud>();
        if (!m_hasAttachedToSharedImageMemory) {
            m_spcSharedMemory=odcore::wrapper::SharedMemoryFactory::attachToSharedMemory(m_spc.getName()); // Attach the shared point cloud to the shared memory.
            m_hasAttachedToSharedImageMemory = true; 
        } 
        if (m_spcSharedMemory.get() != NULL && m_spcSharedMemory->isValid()) {
            // Using a scoped lock to lock and automatically unlock a shared memory segment.
            odcore::base::Lock lv(m_spcSharedMemory);
            float *spcData = static_cast<float*>(m_spcSharedMemory->getSharedMemory());
            uint32_t startID=0;
            for(uint32_t counter=0;counter<m_spc.getWidth();counter++){
                m_xSpc.push_back(spcData[startID]);
                m_ySpc.push_back(spcData[startID+1]);
                m_zSpc.push_back(spcData[startID+2]);
                startID+=4;
            }
        }
    }
    
    inline void ComparePointCloudModule::clearVectors(){
        m_xCpc.clear();
        m_yCpc.clear();
        m_zCpc.clear();
        m_xSpc.clear();
        m_ySpc.clear();
        m_zSpc.clear();
        m_xError.clear();
        m_yError.clear();
        m_zError.clear();
    }
    
    odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ComparePointCloudModule::body() {
        odcore::io::URL url("file://"+m_recordingFile);
        unique_ptr<Player> player;
        //player = unique_ptr<Player>(new Player(url, AUTO_REWIND, MEMORY_SEGMENT_SIZE, NUMBER_OF_SEGMENTS, THREADING));
        player = unique_ptr<Player>(new Player(url, 0, 2800000, 20, false));
        Container c;
        Container cpcFrame;
        Container spcFrame;
        
        if(m_allFrames){
            /*uint32_t spcFrameNumber=0;
            while (player->hasMoreData()){
                c = player->getNextContainerToBeSent();
                if(c.getDataType() == odcore::data::CompactPointCloud::ID()){
                    m_frameNumber++;
                }
                if(c.getDataType() == odcore::data::SharedPointCloud::ID()){
                    spcFrameNumber++;
                }
            }
            cout<<m_frameNumber<<endl;
            cout<<spcFrameNumber<<endl;
            return 0;*/
            while (player->hasMoreData() && m_frameNumber<3000){
               c = player->getNextContainerToBeSent();
               if(c.getDataType() == odcore::data::SharedPointCloud::ID()){
                    readSPC(c);
                    m_frameNumber++;
                }  
                 
                c = player->getNextContainerToBeSent();
                if(c.getDataType() == odcore::data::CompactPointCloud::ID()){
                    readCPC(c);
                }  
                     
                float error_x,error_y,error_z;
                uint32_t spc_index=0;
                for(uint32_t cpc_index=0;cpc_index<m_xCpc.size();cpc_index++){
                    error_x=abs(m_xCpc[cpc_index]-m_xSpc[spc_index]);
                    error_y=abs(m_yCpc[cpc_index]-m_ySpc[spc_index]);
                    error_z=abs(m_zCpc[cpc_index]-m_zSpc[spc_index]);
                    if(error_z<0.003){
                        m_xError.push_back(error_x);
                        m_yError.push_back(error_y);
                        m_zError.push_back(error_z);
                        spc_index++;
                    }
                }
                 
                float max_xError=m_xError[0];
                float sum_xError=0.0;
                for(float i : m_xError){
                    sum_xError+=i;
                    if(i>max_xError){
                        max_xError=i;
                    }
                }
                float avg_xError=sum_xError/m_xError.size();
               
                float max_yError=m_yError[0];
                float sum_yError=0.0;
                for(float i : m_yError){
                    sum_yError+=i;
                    if(i>max_yError){
                        max_yError=i;
                    }
                }
                float avg_yError=sum_yError/m_yError.size();
                
                float max_zError=m_zError[0];
                float sum_zError=0.0;
                for(float i : m_zError){
                    sum_zError+=i;
                    if(i>max_zError){
                        max_zError=i;
                    }
                }
                float avg_zError=sum_zError/m_zError.size();
                if(m_xError.size()==m_xSpc.size() && m_yError.size()==m_ySpc.size() && 
                m_zError.size()==m_zSpc.size()){
                    m_outputData<<max_xError<<","<<avg_xError<<","<<max_yError<<","<<avg_yError<<","
                    <<max_zError<<","<<avg_zError<<endl;  
                }
                else{
                   cout<<"Abnormal frame:"<<m_frameNumber<<endl;
                }
                clearVectors();   
            }
        }
        else{
            /*if(player->hasMoreData()){
                //CPC has one more frame than SPC. Discard the first frame of CPC
                c = player->getNextContainerToBeSent();
            }*/
            while(player->hasMoreData() && m_currentFrame<m_chosenFrame){
                c = player->getNextContainerToBeSent();
                c = player->getNextContainerToBeSent();
                m_currentFrame++;
            }
            
            if(m_currentFrame<m_chosenFrame){
                cerr<<"The chosen frame number is too large for the recording. Choose a smaller frame."<<endl;
            }
            else{
                //Compare x, y, z of the chosen frame between CPC and SPC
                spcFrame = player->getNextContainerToBeSent();
                if(spcFrame.getDataType() == odcore::data::SharedPointCloud::ID()){
                    readSPC(spcFrame);
                }  
                cout<<"Number of points of Frame "<<m_chosenFrame<<" of SPC:"<<m_xSpc.size()<<endl;
                
                cpcFrame = player->getNextContainerToBeSent();
                if(cpcFrame.getDataType() == odcore::data::CompactPointCloud::ID()){
                    readCPC(cpcFrame);
                }  
                cout<<"Number of points of Frame "<<m_chosenFrame<<" of CPC:"<<m_xCpc.size()<<endl;
                
                cout<<"CPC points:"<<m_xCpc.size()<<endl;
                cout<<"SPC points:"<<m_xSpc.size()<<endl;
                //cout<<"The first 20 points of CPC:"<<endl;
                for(uint64_t index=0;index<m_xCpc.size();index++){
                    m_cpcFrame<<m_xCpc[index]<<","<<m_yCpc[index]<<","<<m_zCpc[index]<<endl;   
                }
                
                //cout<<"The first 20 points of SPC:"<<endl;
                for(uint64_t index=0;index<m_xSpc.size();index++){
                    m_spcFrame<<m_xSpc[index]<<","<<m_ySpc[index]<<","<<m_zSpc[index]<<endl;   
                }
                    
                float error_x,error_y,error_z;
                uint32_t spc_index=0;
                for(uint32_t cpc_index=0;cpc_index<m_xCpc.size();cpc_index++){
                    error_x=abs(m_xCpc[cpc_index]-m_xSpc[spc_index]);
                    error_y=abs(m_yCpc[cpc_index]-m_ySpc[spc_index]);
                    error_z=abs(m_zCpc[cpc_index]-m_zSpc[spc_index]);
                    //For the same point in CPC and SPC, the difference in z should be smaller
                    //than 0.003
                    //Max distance error: 0.8cm (original integer value divided by 5 to get cm
                    //while storing CPC), i.e., 0.008m. Hence, the maximum difference in z should
                    //be 0.008 * sin(verticalAngle * toRadian), while verticalAngle is from -15 to 
                    //15 with increment 2
                    if(error_z<0.003){
                        m_outputData<<error_x<<","<<error_y<<","<<error_z<<endl;
                        spc_index++;
                    }
                }
                
                m_xCpc.clear();
                m_yCpc.clear();
                m_zCpc.clear();
                m_xSpc.clear();
                m_ySpc.clear();
                m_zSpc.clear();  
            }
        }
        
        while (getModuleStateAndWaitForRemainingTimeInTimeslice() ==  odcore::data::dmcp::ModuleStateMessage::RUNNING){
            if(m_chosenFrame==m_currentFrame){
                getConference().send(spcFrame);
                getConference().send(cpcFrame);
            }
        } 
        
        return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
    }

} // odcomparepointcloud

/**
 * odcomparepointcloud - Tool for comparing data between shared point cloud and 
 * compact point cloud
 * Copyright (C) 2016 Hang
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

#include "opendavinci/odtools/player/Player.h"
#include "opendavinci/odcore/base/Lock.h"
#include "opendavinci/odcore/wrapper/SharedMemoryFactory.h"

#include "ComparePointCloud.h"

namespace odcomparepointcloud {

    using namespace std;
    using namespace odcore::data;
    using namespace odtools::player;

    ComparePointCloud::ComparePointCloud() :
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
        outputData("output.csv", std::ios_base::app | std::ios_base::out){
        
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

    ComparePointCloud::~ComparePointCloud() {}

    int32_t ComparePointCloud::run() {
        odcore::io::URL url("file://recording.rec");
        unique_ptr<Player> player;
        //player = unique_ptr<Player>(new Player(url, AUTO_REWIND, MEMORY_SEGMENT_SIZE, NUMBER_OF_SEGMENTS, THREADING));
        player = unique_ptr<Player>(new Player(url, 0, 2800000, 20, false));
        Container c;

        //while (player->hasMoreData()){
        //Compare x, y, z of the first frame between CPC and SPC
            //CPC has one more frame than SPC. Discard the first frame of CPC
            c = player->getNextContainerToBeSent();
            
            c = player->getNextContainerToBeSent();
            
            if(c.getDataType() == odcore::data::SharedPointCloud::ID()){
                //cout<<"SPC container found!"<<endl;
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
            
        cout<<"Number of points of the 1st frame of SPC:"<<m_xSpc.size()<<endl;
        
        c = player->getNextContainerToBeSent();
            if(c.getDataType() == odcore::data::CompactPointCloud::ID()){
                //cout<<"CPC container found!"<<endl;
                m_cpc = c.getData<CompactPointCloud>();  
                float startAzimuth = m_cpc.getStartAzimuth();
                float endAzimuth = m_cpc.getEndAzimuth();
                uint8_t entriesPerAzimuth = m_cpc.getEntriesPerAzimuth();
                string distances = m_cpc.getDistances();
                uint32_t numberOfPoints = distances.size()/2;
                uint32_t numberOfAzimuths = numberOfPoints/entriesPerAzimuth;
                float azimuthIncrement = (endAzimuth-startAzimuth)/numberOfAzimuths;//Calculate the azimuth increment
                stringstream sstr(distances);
                //const float toRadian = static_cast<float>(cartesian::Constants::PI) / 180.0f;
                const float toRadian = static_cast<float>(M_PI) / 180.0f;
                uint16_t distance_integer=0;
                float xyDistance = 0;
                float azimuth = startAzimuth;
                for (uint32_t azimuthIndex = 0; azimuthIndex < numberOfAzimuths; azimuthIndex++) {
                    //float verticalAngle = START_V_ANGLE;
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
                        //verticalAngle += V_INCREMENT;
                    }
                    azimuth+=azimuthIncrement;
                }
            }
            
            cout<<"Number of points of the 1st frame of CPC:"<<m_xCpc.size()<<endl;
            
            float error_x,error_y,error_z;
            if(m_xCpc.size()==m_xSpc.size()){
               for(uint32_t index=0;index<m_xCpc.size();index++){
                    error_x=abs(m_xCpc[index]-m_xSpc[index]);
                    error_y=abs(m_yCpc[index]-m_ySpc[index]);
                    error_z=abs(m_zCpc[index]-m_zSpc[index]);
                    outputData<<error_x<<","<<error_y<<","<<error_z<<endl;
                }
                /*cout<<"The first 20 points of CPC:"<<endl;
                for(uint32_t index=0;index<20;index++){
                    cout<<m_xCpc[index]<<","<<m_yCpc[index]<<","<<m_zCpc[index]<<endl;   
                }
                
                cout<<"The first 20 points of SPC:"<<endl;
                for(uint32_t index=0;index<20;index++){
                    cout<<m_xSpc[index]<<","<<m_ySpc[index]<<","<<m_zSpc[index]<<endl;   
                }*/
            }
        //}
        
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
        cout<<spcFrameNumber<<endl;  */
        
        //m_frameNumber=m_frameNumber / 2;
        //cout<<"Number of frames:"<<m_frameNumber<<endl;
        
        
        return 0;
    }

} // odcomparepointcloud
//vector.clear();

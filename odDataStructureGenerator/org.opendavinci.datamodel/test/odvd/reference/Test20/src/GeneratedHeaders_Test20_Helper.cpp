/*
 * THIS IS A GENERATED FILE - CHANGES WILL BE OVERWRITTEN.
 */

#include <iostream>
#include <test20/GeneratedHeaders_Test20.h>
#include <test20/GeneratedHeaders_Test20_Helper.h>
#include <opendavinci/odcore/base/Visitable.h>
#include <opendavinci/odcore/reflection/MessageFromVisitableVisitor.h>
void GeneratedHeaders_Test20_Helper::delegateVistor(odcore::data::Container &c, odcore::base::Visitor &v, bool &successfullyDelegated) {
    successfullyDelegated = false;
    if (c.getDataType() == Test20a::ID()) {
        Test20a payload = c.getData<Test20a>();
        payload.accept(v);
        successfullyDelegated = true;
    }
    if (c.getDataType() == sub::structure::Test20b::ID()) {
        sub::structure::Test20b payload = c.getData<sub::structure::Test20b>();
        payload.accept(v);
        successfullyDelegated = true;
    }
    if (c.getDataType() == sub::structure2::Test20c::ID()) {
        sub::structure2::Test20c payload = c.getData<sub::structure2::Test20c>();
        payload.accept(v);
        successfullyDelegated = true;
    }
}
odcore::reflection::Message GeneratedHeaders_Test20_Helper::map(odcore::data::Container &c, bool &successfullyMapped) {
    successfullyMapped = false;
    odcore::reflection::Message msg;
    odcore::reflection::MessageFromVisitableVisitor mfvv;
    delegateVistor(c, mfvv, successfullyMapped);
    if (successfullyMapped) {
        msg = mfvv.getMessage();
    }
    return msg;
}

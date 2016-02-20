/*
 * This software is open source. Please see COPYING and AUTHORS for further information.
 *
 * This file is auto-generated. DO NOT CHANGE AS YOUR CHANGES MIGHT BE OVERWRITTEN!
 */


#include "opendavinci/odcore/base/Hash.h"
#include "opendavinci/odcore/base/Deserializer.h"
#include "opendavinci/odcore/base/SerializationFactory.h"
#include "opendavinci/odcore/base/Serializer.h"


#include "test19/generated/Test19b.h"

	using namespace std;
	using namespace odcore::base;


	Test19b::Test19b() :
	    SerializableData(), Visitable()
	{
	}


	Test19b::Test19b(const Test19b &obj) :
	    SerializableData(), Visitable()
	{
		(void)obj; // Avoid unused parameter warning.
	}
	
	Test19b::~Test19b() {
	}

	Test19b& Test19b::operator=(const Test19b &obj) {
		(void)obj; // Avoid unused parameter warning.
		return (*this);
	}

	int32_t Test19b::ID() {
		return 20;
	}

	const string Test19b::ShortName() {
		return "Test19b";
	}

	const string Test19b::LongName() {
		return "Test19b";
	}


	void Test19b::accept(odcore::base::Visitor &v) {
		(void)v; // Avoid unused parameter warning.
	}

	const string Test19b::toString() const {
		stringstream s;



		return s.str();
	}

	ostream& Test19b::operator<<(ostream &out) const {

		SerializationFactory& sf = SerializationFactory::getInstance();

		odcore::SharedPointer<Serializer> s = sf.getSerializer(out);(void)s; // Avoid unused variable warning.

		return out;
	}

	istream& Test19b::operator>>(istream &in) {

		SerializationFactory& sf = SerializationFactory::getInstance();

		odcore::SharedPointer<Deserializer> d = sf.getDeserializer(in);(void)d; // Avoid unused variable warning.

		return in;
	}

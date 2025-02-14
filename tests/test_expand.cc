/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file   test_MetFile.cc
/// @date   Jan 2016
/// @author Florian Rathgeber

#include "eckit/types/Date.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/MarsExpension.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Type.h"

#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit {
namespace mars {
namespace test {

//-----------------------------------------------------------------------------

CASE( "test_metkit_expand_1" ) {
    const char* text = "ret,date=-5/to/-1";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}


CASE( "test_metkit_expand_2" ) {
    const char* text = "ret";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);

    const std::vector< std::string >& dates = r.values("date");
    EXPECT(dates.size() == 1);

    eckit::Date today(0);
    std::ostringstream oss;

    oss << today.yyyymmdd();
    EXPECT(dates[0] == oss.str());


}

CASE( "test_metkit_expand_3" ) {
    const char* text = "ret,date=-5/to/-1,grid=n640";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}


CASE( "test_metkit_expand_4" ) {
    const char* text = "ret,date=-5/to/-1,grid=o640";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}

CASE( "test_metkit_expand_5" ) {
    const char* text = "retrieve,class=od,date=20050601,diagnostic=1,expver=1,iteration=0,levelist=1,levtype=ml,param=155.129,step=0,stream=sens,time=1200,type=sg";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}

CASE( "test_metkit_expand_6" ) {
    const char* text = "retrieve,class=rd,expver=hl1m,stream=oper,date=20000801,time=0000,domain=g,type=fc,levtype=pl,step=24,param=129,levelist=1/to/31";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}

CASE( "test_metkit_expand_7" ) {
    const char* text = "retrieve,class=rd,expver=hl1m,stream=oper,date=20000801,time=0000,domain=g,type=fc,levtype=pl,step=24,param=129,levelist=0.01/0.7";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}

CASE( "test_metkit_expand_8" ) {
    const char* text = "retrieve,class=rd,expver=hl1m,stream=oper,date=20000801,time=0000,domain=g,type=fc,levtype=pl,step=24,param=129,levelist=0.1/to/0.7/by/0.2";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}

CASE( "test_metkit_expand_9_strict" ) {
    const char* text = "retrieve,class=rd,expver=hm1u,stream=weeh,time=0000,date=20210101,domain=g,hdate=20190101";
    {
        std::istringstream in(text);
        MarsParser parser(in);
        MarsExpension expand(false, false);
        std::vector<MarsRequest> v = expand.expand(parser.parse());

        ASSERT(v.size() == 1);
        v[0].dump(std::cout);
    }
    {
        std::istringstream in(text);
        MarsParser parser(in);
        MarsExpension expand(false, true);
        std::vector<MarsRequest> v = expand.expand(parser.parse());

        ASSERT(v.size() == 1);
        v[0].dump(std::cout);
    }
}

CASE( "test_metkit_expand_10_strict" ) {
    const char* text = "retrieve,class=rd,expver=hm1u,stream=wees,time=0000,date=20210101,domain=g,hdate=20190101";
    {
        std::istringstream in(text);
        MarsParser parser(in);
        MarsExpension expand(false, false);
        std::vector<MarsRequest> v = expand.expand(parser.parse());

        ASSERT(v.size() == 1);
        v[0].dump(std::cout);
    }
    {
        std::istringstream in(text);
        MarsParser parser(in);
        MarsExpension expand(false, true);
        EXPECT_THROWS(expand.expand(parser.parse()));
    }
}

void quantileThrows(std::vector<std::string> values) {
    DummyContext ctx;
    static metkit::mars::MarsLanguage language("retrieve");
    metkit::mars::Type* t = language.type("quantile");
    EXPECT_THROWS_AS(t->expand(ctx, values), eckit::BadValue);
}

void quantile(std::vector<std::string> values, std::vector<std::string> expected) {
    DummyContext ctx;
    static metkit::mars::MarsLanguage language("retrieve");
    metkit::mars::Type* t = language.type("quantile");
    t->expand(ctx, values);
    ASSERT(values == expected);
}

CASE( "test_metkit_expand_11_quantile" ) {
    quantileThrows({"-1:5"});
    quantileThrows({"0:-5"});
    quantileThrows({"6:5"});
    quantileThrows({"0:12"});
    quantile({"2:5"}, {"2:5"});
    quantile({"0:2","1:2","2:2"}, {"0:2","1:2","2:2"});
    quantile({"0:2","1:3","2:5"}, {"0:2","1:3","2:5"});

    quantileThrows({"to","5:10"});
    quantileThrows({"3:5","to"});
    quantileThrows({"3:5","to","5:10"});
    quantileThrows({"3:5","to","2:5"});
    quantileThrows({"1:5","to","3:5","by"});
    quantileThrows({"1:5","to","3:5","by","1:5"});

    quantile({"0:5","to","0:5"}, {"0:5"});
    quantile({"3:3","to","3:3"}, {"3:3"});
    quantile({"0:5","to","5:5"}, {"0:5","1:5","2:5","3:5","4:5","5:5"});
    quantile({"0:5","to","5:5","by","1"}, {"0:5","1:5","2:5","3:5","4:5","5:5"});
    quantile({"0:5","to","5:5","by","2"}, {"0:5","2:5","4:5"});
    quantile({"0:5","to","5:5","by","3"}, {"0:5","3:5"});
    quantile({"0:5","to","5:5","by","5"}, {"0:5","5:5"});
    quantile({"0:5","to","5:5","by","6"}, {"0:5"});
    quantile({"2:5","to","5:5","by","2"}, {"2:5","4:5"});
    quantile({"3:5","to","5:5","by","2"}, {"3:5","5:5"});
    quantile({"4:5","to","5:5","by","2"}, {"4:5"});
    quantile({"0:10","3:10","to","7:10","by","2","10:10"}, {"0:10","3:10","5:10","7:10","10:10"});
}

//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace mars
}  // namespace metkit

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}

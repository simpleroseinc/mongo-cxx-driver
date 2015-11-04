#include "catch.hpp"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>

using namespace bsoncxx;

TEST_CASE("[] can reach into nested arrays", "[bsoncxx]") {
    using namespace builder::stream;

    builder::stream::document build_doc;
    // {
    //     "ints": [ 1, 3, [ 5 ] ],
    //     "bools": [ true, false ]
    // }
    build_doc << "ints" << open_array << 1 << 3 << open_array << 5 << close_array << close_array
              << "bools" << open_array << true << false << close_array;

    auto doc = build_doc.view();

    SECTION("works with one level") {
        REQUIRE(doc["ints"][0]);
        REQUIRE(doc["ints"][0].get_int32() == 1);

        REQUIRE(doc["ints"][1]);
        REQUIRE(doc["ints"][1].get_int32() == 3);

        REQUIRE(doc["bools"][0]);
        REQUIRE(doc["bools"][0].get_bool() == true);

        REQUIRE(doc["bools"][1]);
        REQUIRE(doc["bools"][1].get_bool() == false);
    }

    SECTION("works with two levels") {
        REQUIRE(doc["ints"][2][0]);
        REQUIRE(doc["ints"][2][0].get_int32() == 5);
    }

    SECTION("throws on non-array") {
        REQUIRE_THROWS(doc["ints"][0][0]);
        REQUIRE_THROWS(doc["ints"][2][0][0]);
        REQUIRE_THROWS(doc["bools"][1][0]);
    }

    SECTION("returns invalid on out-of-bounds") {
        REQUIRE(!doc["ints"][9]);
        REQUIRE(!doc["ints"][2][9]);
        REQUIRE(!doc["bools"][9]);
    }

}

TEST_CASE("[] can reach into nested documents", "[bsoncxx]") {
    using namespace builder::stream;

    builder::stream::document build_doc;
    // {
    //     "ints": {
    //         "x": 1,
    //         "y": 3,
    //         "more": {
    //             "z": 5
    //         }
    //     },
    //     "bools": {
    //         "t": true,
    //         "f": false,
    //     }
    // }
    build_doc << "ints" << open_document << "x" << 1 << "y" << 3 << "more" << open_document << "z"
              << 5 << close_document << close_document << "bools" << open_document << "t" << true
              << "f" << false << close_document;

    auto doc = build_doc.view();

    SECTION("works with one level") {
        REQUIRE(doc["ints"]["x"]);
        REQUIRE(doc["ints"]["x"].get_int32() == 1);

        REQUIRE(doc["ints"]["y"]);
        REQUIRE(doc["ints"]["y"].get_int32() == 3);

        REQUIRE(doc["bools"]["t"]);
        REQUIRE(doc["bools"]["t"].get_bool() == true);

        REQUIRE(doc["bools"]["f"]);
        REQUIRE(doc["bools"]["f"].get_bool() == false);
    }

    SECTION("works with two levels") {
        REQUIRE(doc["ints"]["more"]["z"]);
        REQUIRE(doc["ints"]["more"]["z"].get_int32() == 5);
    }

    SECTION("throws on non-document") {
        REQUIRE_THROWS(doc["ints"]["x"]["key"]);
        REQUIRE_THROWS(doc["ints"]["more"]["z"]["key"]);
        REQUIRE_THROWS(doc["bools"]["f"]["key"]);
    }

    SECTION("returns invalid on not found") {
        REQUIRE(!doc["ints"]["badKey"]);
        REQUIRE(!doc["ints"]["more"]["badKey"]);
        REQUIRE(!doc["bools"]["badKey"]);
    }
}

TEST_CASE("[] can reach into mixed nested arrays and documents", "[bsoncxx]") {
    using namespace builder::stream;

    builder::stream::document build_doc;
    // {
    //     "ints": {
    //         "x": 1,
    //         "y": 3,
    //         "arr": [
    //             5,
    //             7,
    //             {
    //                 "z": 9,
    //                 "even_more": [ 11 ]
    //             }
    //         ]
    //     },
    //     "bools": {
    //         "t": true,
    //         "f": false,
    //         "arr": [ false, true ]
    //     }
    // }
    build_doc << "ints" << open_document << "x" << 1 << "y" << 3 << "arr" << open_array << 5 << 7
              << open_document << "z" << 9 << "even_more" << open_array << 11 << close_array
              << close_document << close_array << close_document << "bools" << open_document << "t"
              << true << "f" << false << "arr" << open_array << false << true << close_array
              << close_document;

    auto doc = build_doc.view();

    SECTION("succeeds on ints") {
        REQUIRE(doc["ints"]["x"].get_int32() == 1);
        REQUIRE(doc["ints"]["y"].get_int32() == 3);
        REQUIRE(doc["ints"]["arr"][0].get_int32() == 5);
        REQUIRE(doc["ints"]["arr"][1].get_int32() == 7);
        REQUIRE(doc["ints"]["arr"][2]["z"].get_int32() == 9);
        REQUIRE(doc["ints"]["arr"][2]["even_more"][0].get_int32() == 11);
    }

    SECTION("succeeds on bools") {
        REQUIRE(doc["bools"]["t"].get_bool() == true);
        REQUIRE(doc["bools"]["f"].get_bool() == false);
        REQUIRE(doc["bools"]["arr"][0].get_bool() == false);
        REQUIRE(doc["bools"]["arr"][1].get_bool() == true);
    }

    SECTION("throws on invalid") {
        REQUIRE_THROWS(doc["ints"]["badKey"]["anything"]);
        REQUIRE_THROWS(doc["ints"]["badKey"][0]);
        REQUIRE_THROWS(doc["ints"]["arr"][99]["anything"]);
        REQUIRE_THROWS(doc["ints"]["arr"][99][0]);
    }
}

TEST_CASE("[] with large nesting levels", "[bsoncxx]") {
    using namespace builder::stream;

    std::int32_t nesting_level;

    SECTION("no nesting") {
        nesting_level = 0;
    }

    SECTION("2 nesting level") {
        nesting_level = 2;
    }

    SECTION("100 nesting level") {
        nesting_level = 100;
    }

    SECTION("200 nesting level") {
        nesting_level = 200;
    }

    SECTION("2000 nesting level") {
        nesting_level = 2000;
    }

    builder::stream::document build_doc;

    for (int i = 0; i < nesting_level; ++i) {
        build_doc << "x" << open_document;
    }

    build_doc << "x" << nesting_level;

    for (int i = 0; i < nesting_level; ++i) {
        build_doc << close_document;
    }

    auto x = build_doc.view()["x"];
    for (int i = 0; i < nesting_level; ++i) {
        REQUIRE(x["x"]);
        x = x["x"];
    }
    REQUIRE(x.get_int32() == nesting_level);
}

#include <pajlada/settings.hpp>

#include "test/common.hpp"

using namespace pajlada::Settings;

#ifdef PAJLADA_BOOST_ANY_SUPPORT
TEST(Map, Simple)
{
    using boost::any_cast;

    Setting<std::map<std::string, boost::any>> test("/map");

    EXPECT_TRUE(LoadFile("in.simplemap.json"));

    auto myMap = test.getValue();
    EXPECT_TRUE(myMap.size() == 3);
    EXPECT_TRUE(any_cast<int>(myMap["a"]) == 1);
    EXPECT_TRUE(any_cast<std::string>(myMap["b"]) == "asd");
    EXPECT_TRUE(any_cast<double>(myMap["c"]) == 3.14);

    std::vector<std::string> keys{"a", "b", "c"};

    EXPECT_TRUE(keys == SettingManager::getObjectKeys("/map"));

    EXPECT_TRUE(SettingManager::gSaveAs("files/out.simplemap.json"));
}

TEST(Map, Complex)
{
    using boost::any_cast;

    Setting<std::map<std::string, boost::any>> test("/map");

    EXPECT_TRUE(LoadFile("in.complexmap.json"));

    auto myMap = test.getValue();
    EXPECT_TRUE(myMap.size() == 3);
    EXPECT_TRUE(any_cast<int>(myMap["a"]) == 5);

    auto innerMap =
        any_cast<std::map<std::string, boost::any>>(myMap["innerMap"]);
    EXPECT_TRUE(innerMap.size() == 3);
    EXPECT_TRUE(any_cast<int>(innerMap["a"]) == 420);
    EXPECT_TRUE(any_cast<int>(innerMap["b"]) == 320);
    EXPECT_TRUE(any_cast<double>(innerMap["c"]) == 13.37);

    auto innerArray = any_cast<std::vector<boost::any>>(myMap["innerArray"]);
    EXPECT_TRUE(innerArray.size() == 9);
    EXPECT_TRUE(any_cast<int>(innerArray[0]) == 1);
    EXPECT_TRUE(any_cast<int>(innerArray[1]) == 2);
    EXPECT_TRUE(any_cast<int>(innerArray[2]) == 3);
    EXPECT_TRUE(any_cast<int>(innerArray[3]) == 4);
    EXPECT_TRUE(any_cast<std::string>(innerArray[4]) == "testman");
    EXPECT_TRUE(any_cast<bool>(innerArray[5]) == true);
    EXPECT_TRUE(any_cast<bool>(innerArray[6]) == false);
    EXPECT_TRUE(any_cast<double>(innerArray[7]) == 4.20);

    auto innerArrayMap =
        any_cast<std::map<std::string, boost::any>>(innerArray[8]);
    EXPECT_TRUE(innerArrayMap.size() == 3);
    EXPECT_TRUE(any_cast<int>(innerArrayMap["a"]) == 1);
    EXPECT_TRUE(any_cast<int>(innerArrayMap["b"]) == 2);
    EXPECT_TRUE(any_cast<int>(innerArrayMap["c"]) == 3);

    EXPECT_TRUE(SettingManager::gSaveAs("files/out.complexmap.json"));
}
#endif

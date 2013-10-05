#include "gtest/gtest.h"

#include "textile.h"

#include <string>
#include <sstream>
#include <iterator>
#include <fstream>

using std::string;
using std::ostream_iterator;
using std::istreambuf_iterator;
using std::ostringstream;
using std::copy;
using std::ifstream;

namespace {

    string ifstream_to_string(ifstream &strm) {
        return string(
                istreambuf_iterator<char>(strm),
                istreambuf_iterator<char>());
    }

    struct TextileHelper {
        ostringstream stream;

        void merged(string str) { stream << str; }

        void conflicted( string base, string ours, string theirs) {
            stream << "<<<<<<<" << ours << "|||||||" << base
                << "=======" << theirs << ">>>>>>>";
        }

        static void merged_callback(void *data, const char *str, size_t size) {
            TextileHelper *instance = reinterpret_cast<TextileHelper*>(data);

            instance->merged(string(str, size));
        }

        static void conflict_callback(void *data,
                const char *base, size_t base_size,
                const char *ours, size_t ours_size,
                const char *theirs, size_t theirs_size
                ) {
            TextileHelper *instance = reinterpret_cast<TextileHelper*>(data);

            instance->conflicted(string(base, base_size),
                                 string(ours, ours_size),
                                 string(theirs, theirs_size ));
        }

        bool call_textile_merge( string base, string ours, string theirs) {
            return textile_merge(
                    base.c_str(), base.length(),
                    ours.c_str(), ours.length(),
                    theirs.c_str(), theirs.length(),

                    TextileHelper::merged_callback,
                    TextileHelper::conflict_callback,

                    reinterpret_cast<void*>(this));
        }

        bool call_textile_merge( ifstream &base, ifstream &ours, ifstream &theirs) {
            return call_textile_merge(
                    ifstream_to_string(base),
                    ifstream_to_string(ours),
                    ifstream_to_string(theirs)
                    );
        }
    };

    class TextileTest : public ::testing::Test {
        protected:
            TextileTest() {}
            virtual ~TextileTest() {}
            virtual void SetUp() {}
            virtual void TearDown() {}

            TextileHelper merge;
    };

    TEST_F(TextileTest, TestEmpty) {
        bool rc = merge.call_textile_merge( "", "", "");

        ASSERT_FALSE(rc);
        ASSERT_EQ("", merge.stream.str());
    }

    TEST_F(TextileTest, TestTheyDelete) {
        ASSERT_FALSE(merge.call_textile_merge( "deleteme", "deleteme", "" ));
        ASSERT_EQ("", merge.stream.str());
    }

    TEST_F(TextileTest, TestWeDelete) {
        ASSERT_FALSE(merge.call_textile_merge( "deleteme", "", "deleteme" ));
        ASSERT_EQ("", merge.stream.str());
    }

    TEST_F(TextileTest, TestWeAdd) {
        ASSERT_FALSE(merge.call_textile_merge( "", "addme", "" ));
        ASSERT_EQ("addme", merge.stream.str());
    }

    TEST_F(TextileTest, TestTheyAdd) {
        ASSERT_FALSE(merge.call_textile_merge( "", "", "addme" ));
        ASSERT_EQ("addme", merge.stream.str());
    }

    TEST_F(TextileTest, TestBothAdd) {
        ASSERT_FALSE(merge.call_textile_merge( "", "addme", "addme" ));
        ASSERT_EQ("addme", merge.stream.str());
    }

    TEST_F(TextileTest, TestTinyMerge) {
        bool rc = merge.call_textile_merge(
                "A shrt strang.",
                "A short strang.",
                "A shrt string.");

        ASSERT_FALSE(rc);
        ASSERT_EQ("A short string.", merge.stream.str());
    }

    TEST_F(TextileTest, TestNoBase) {
        bool rc = merge.call_textile_merge(
            "",

            "Content we added.",

            "Content they added."
            );

        ASSERT_TRUE(rc);
        ASSERT_EQ(
                "<<<<<<<Content we added.|||||||=======Content they added.>>>>>>>",
                merge.stream.str());
    }

    TEST_F(TextileTest, DISABLED_TestNoBaseBetter) {
        bool rc = merge.call_textile_merge(
            "",
            "Content we added.",
            "Content they added."
            );

        ASSERT_TRUE(rc);
        ASSERT_EQ(
                "Content <<<<<<<we|||||||=======they>>>>>>> added.",
                merge.stream.str());
    }

    TEST_F(TextileTest, TestChangeAtEol) {
        bool rc = merge.call_textile_merge(
            "Etiam at felis quis leo feugiat suscipit.",
            "Etiam at felis quis leo feugiat suscipit?",
            "Etiam at felis quis leo feugiat suscipit!"
            );

        ASSERT_TRUE(rc);
        ASSERT_EQ(
                "Etiam at felis quis leo feugiat suscipit<<<<<<<?|||||||.=======!>>>>>>>",
                merge.stream.str());
    }

    TEST_F(TextileTest, TestSimple) {
        bool rc = merge.call_textile_merge(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"
            "Nam nec massa tincidunt, consectetur nunc in, commodo dui.\n",

            "Lorem ipsum color sit amen, consectetur adipiscing elit.\n"
            "Name nec massa tincidunt, consectetur nunc in, commode dui.\n",

            "Lorem ipsum dolor set amet, consectur adipiscing elite.\n"
            "Nam nec mass tincidunt, consectetur nunc in, commodo dui.\n"
            );

        ASSERT_FALSE(rc);
        ASSERT_EQ(
            "Lorem ipsum color set amen, consectur adipiscing elite.\n"
            "Name nec mass tincidunt, consectetur nunc in, commode dui.\n",
                merge.stream.str());
    }

    TEST_F(TextileTest, TestInsertBeforeDelete) {
        bool rc = merge.call_textile_merge(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit.",
            "Lorem ipsum dolor sit amet, adipiscing elit.",
            "Lorem ipsum dolor sit amet, insert consectetur adipiscing elit."
            );

        ASSERT_EQ(
            "Lorem ipsum dolor sit amet, insert adipiscing elit.",
                merge.stream.str());
        ASSERT_FALSE(rc);
    }

    TEST_F(TextileTest, TestInsertAfterDelete) {
        bool rc = merge.call_textile_merge(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit.",
            "Lorem ipsum dolor sit amet, adipiscing elit.",
            "Lorem ipsum dolor sit amet, consectetur insert adipiscing elit."
            );

        ASSERT_EQ(
            "Lorem ipsum dolor sit amet, insert adipiscing elit.",
                merge.stream.str());
        ASSERT_FALSE(rc);
    }

    TEST_F(TextileTest, TestInsertAndChange) {
        bool rc = merge.call_textile_merge(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit.",
            "Lorem ipsum dolor sit amet, change adipiscing elit.",
            "Lorem ipsum dolor sit amet, consecteturd adipiscing elit."
            );

        ASSERT_EQ(
            "Lorem ipsum dolor sit amet, changed adipiscing elit.",
                merge.stream.str());
        ASSERT_FALSE(rc);
    }


    TEST_F(TextileTest, TestAllMergeTypes) {
        ifstream base("data/AllMergeTypes/base");
        ifstream ours("data/AllMergeTypes/ours");
        ifstream theirs("data/AllMergeTypes/theirs");

        bool rc = merge.call_textile_merge(base, ours, theirs);

        ASSERT_TRUE(rc);

        ifstream golden("data/AllMergeTypes/golden");
        ASSERT_EQ(ifstream_to_string(golden), merge.stream.str());
    }

    TEST_F(TextileTest, TestTrickyMerge) {
        ifstream base("data/TrickyMerge/base");
        ifstream ours("data/TrickyMerge/ours");
        ifstream theirs("data/TrickyMerge/theirs");

        bool rc = merge.call_textile_merge(base, ours, theirs);

        ASSERT_FALSE(rc);

        ifstream golden("data/TrickyMerge/golden");
        ASSERT_EQ(ifstream_to_string(golden), merge.stream.str());
    }

    TEST_F(TextileTest, TestOnlyDeletes) {
        ifstream base("data/OnlyDeletes/base");
        ifstream ours("data/OnlyDeletes/ours");
        ifstream theirs("data/OnlyDeletes/theirs");

        bool rc = merge.call_textile_merge(base, ours, theirs);

        ASSERT_FALSE(rc);

        ifstream golden("data/OnlyDeletes/golden");
        ASSERT_EQ(ifstream_to_string(golden), merge.stream.str());
    }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

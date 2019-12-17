//
// Created by Egor Orachyov on 2019-10-26.
//

#ifndef IGNIMBRITELIBRARY_TESTOBJECTID_CPP
#define IGNIMBRITELIBRARY_TESTOBJECTID_CPP

#include <ignimbrite/ObjectIDBuffer.h>
#include <ignimbrite/FileUtils.h>
#include <string>

using namespace ignimbrite;

struct TestObjectIDBuffer {

    static void test1() {
        ObjectIDBuffer<std::string> strings;

        auto s1id = strings.add("hello");
        auto s2id = strings.add("world");

        std::string &s1 = strings.get(s1id);
        std::string &s2 = strings.get(s2id);

        std::cout << "Message: " << s1 << "," << s2 << "!\n";

        strings.remove(s1id);
        strings.remove(s2id);
    }

    static void test2() {
        try {
            ObjectIDBuffer<std::string> strings;
            auto id = strings.add("some string");
            strings.get(id);
            strings.remove(id);
            strings.remove(id);
        } catch (std::runtime_error& error) {
            std::cout << "Catch error: " << error.what();
        }
    }

    static void test3() {
        const uint32 count = 1000;
        const uint32 remove = 100;

        ObjectID ids[count];
        ObjectIDBuffer<std::string> strings;

        for (uint32 i = 0; i < count; i++) {
            ids[i] = strings.add("");
        }

        for (uint32 i = 0; i < count; i++) {
            strings.get(ids[i]);
        }

        for (uint32 i = 0; i < remove; i++) {
            strings.remove(ids[i]);
        }

        for (uint32 i = 0; i < count; i++) {
            try {
                strings.get(ids[i]);
            } catch (std::runtime_error& error) {
                std::cout << "ID: " << i << " was removed\n";
            }
        }

        for (uint32 i = remove; i < count; i++) {
            strings.remove(ids[i]);
        }
    }

    static void test4() {

        ObjectIDBuffer<int64> buffer;

        uint32 remove[] { 1, 2, 3, 4, 5, 6 };
        int64 data[] = { 12312, 232, 12312, 213, 123, 77777, 100000 };

        ObjectID ids[sizeof(data)/ sizeof(int64)];

        for (uint32 i = 0; i < sizeof(data)/sizeof(int64); i++) {
            ids[i] = buffer.add(data[i]);
        }

        for (auto toRemove: remove) {
            buffer.remove(ids[toRemove]);
        }

        for (auto& object: buffer) {
            printf("Object: %lli\n", object);
        }

        buffer.remove(ids[0]);
        ids[0] = buffer.add(66666);

        for (auto i = buffer.begin(); i != buffer.end(); ++i) {
            auto id = i.getID();
            printf("Object: %lli id: (%u,%u)\n", *i, id.getIndex(), id.getGeneration());
        }
    }

};

int32 main(int32 argc, char** argv) {
    TestObjectIDBuffer::test1();
    TestObjectIDBuffer::test2();
    TestObjectIDBuffer::test3();
    TestObjectIDBuffer::test4();
}

#endif //IGNIMBRITELIBRARY_TESTOBJECTID_CPP

#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include <string>
#include <filesystem>

#include <csvbuffer/csvmappedbuffer.hpp>

using namespace csv;
const std::string temp_filename = "test_mapped_buffer.tmp";

// Helper fixture to manage temporary files
class MappedBufferTest : public ::testing::Test {
protected:

    void SetUp() override {
        // Ensure clean state
        std::remove(temp_filename.c_str());
    }

    void TearDown() override {
        std::remove(temp_filename.c_str());
    }

    void create_temp_file(const std::string& content, const std::string& filename = temp_filename) {
        std::ofstream out(filename, std::ios::binary);
        out << content;
        out.close();
    }
};

// 1. Basic IO Logic
TEST_F(MappedBufferTest, ReadSimpleContent) {
    std::string content = "Hello, World!";
    create_temp_file(content);

    MappedBuffer buffer(temp_filename);

    EXPECT_TRUE(buffer.good());
    EXPECT_EQ(buffer.available(), content.size());
    
    // View should return everything
    EXPECT_EQ(buffer.view(), content);
}

TEST_F(MappedBufferTest, ConsumeLogic) {
    create_temp_file("ABCDEF");
    MappedBuffer buffer(temp_filename);

    // Initial state
    EXPECT_EQ(buffer.view(), "ABCDEF");

    // Consume part
    buffer.consume(2);
    EXPECT_EQ(buffer.view(), "CDEF");
    EXPECT_EQ(buffer.available(), 4);

    // Consume rest
    buffer.consume(4);
    EXPECT_TRUE(buffer.empty());
    EXPECT_TRUE(buffer.eof());
    EXPECT_EQ(buffer.view(), "");
}

// 2. Interface Compliance
TEST_F(MappedBufferTest, ResetRewindsToStart) {
    std::string content = "12345";
    create_temp_file(content);
    MappedBuffer buffer(temp_filename);

    buffer.consume(3);
    EXPECT_EQ(buffer.view(), "45");

    EXPECT_TRUE(buffer.reset());
    EXPECT_EQ(buffer.view(), "12345");
    EXPECT_EQ(buffer.available(), 5);
}

// 3. Move Semantics (The Critical "Senior" Test)
TEST_F(MappedBufferTest, MoveConstructorTransfersOwnership) {
    create_temp_file("MovedContent");
    
    // Scope to ensure destructors run
    {
        MappedBuffer source(temp_filename);
        EXPECT_TRUE(source.good());

        MappedBuffer target(std::move(source));

        // Target should have the data
        EXPECT_EQ(target.view(), "MovedContent");
        EXPECT_TRUE(target.good());

        // Source should be invalid/empty (but safe to destroy)
        EXPECT_FALSE(source.good());
        EXPECT_EQ(source.view(), "");
    } 
    // If double-free existed, this closing brace would crash (Segfault)
}

TEST_F(MappedBufferTest, MoveAssignmentTransfersOwnership) {
    // 1. Setup specific source file
    std::string source_filename = "source_buffer.tmp";
    create_temp_file("AssignmentContent", source_filename);
    
    MappedBuffer source(source_filename);
    
    // 2. Setup specific target file (using the default fixture filename)
    create_temp_file("Dummy"); 
    MappedBuffer target(temp_filename); 

    // 3. Move
    target = std::move(source);

    // 4. Verification
    // Target should now own the view to "source_buffer.tmp"
    EXPECT_EQ(target.view(), "AssignmentContent"); 
    EXPECT_FALSE(source.good());

    // Cleanup extra file
    std::remove(source_filename.c_str());
}

TEST_F(MappedBufferTest, DemonstrateZeroCopy_LiveFileUpdates) {
    // 1. Create a file with specific content (10 bytes)
    const std::string initial_content = "1111111111";
    create_temp_file(initial_content);

    // 2. Map the file
    // At this point, 'buffer' is just a window into the OS Page Cache
    MappedBuffer buffer(temp_filename);
    EXPECT_EQ(buffer.view(), initial_content);

    // 3. Modify the file externally
    // We modify the file on disk (OS Cache) while the buffer is still open.
    // NOTE: We keep the file size exactly the same to avoid SIGBUS.
    {
        std::fstream disk_modifier(temp_filename, std::ios::in | std::ios::out | std::ios::binary);
        disk_modifier.seekp(0);
        disk_modifier << "99999"; // Overwrite first 5 bytes
        disk_modifier.flush();    // Ensure data hits the OS cache
    }

    // 4. Verify the buffer updated "magically"
    // If MappedBuffer were using malloc/memcpy (standard buffering), 
    // this would still be "1111111111".
    // Because it is mmap, we see the change immediately.
    EXPECT_EQ(buffer.view(), "9999911111");
}


// 4. Edge Cases
TEST_F(MappedBufferTest, ThrowsOnMissingFile) {
    EXPECT_THROW({
        MappedBuffer buffer("non_existent_ghost_file.csv");
    }, std::runtime_error);
}

TEST_F(MappedBufferTest, EmptyFileBehavior) {
    create_temp_file(""); // Empty file
    MappedBuffer buffer(temp_filename);

    // Based on your specific request: Empty file is NOT good
    EXPECT_FALSE(buffer.good());
    EXPECT_TRUE(buffer.eof());
    EXPECT_TRUE(buffer.empty());
    
    // View should be empty string view
    EXPECT_TRUE(buffer.view().empty());
}

TEST_F(MappedBufferTest, ConsumeMoreThanAvailable) {
    create_temp_file("ABC");
    MappedBuffer buffer(temp_filename);

    buffer.consume(100); // Should cap at size
    
    EXPECT_TRUE(buffer.eof());
    EXPECT_EQ(buffer.available(), 0);
}

// 5. Large File Simulation (sanity check)
TEST_F(MappedBufferTest, LargerFileLogic) {
    // 1MB file
    std::string large_content(1024 * 1024, 'X'); 
    create_temp_file(large_content);

    MappedBuffer buffer(temp_filename);
    
    EXPECT_EQ(buffer.available(), 1024 * 1024);
    EXPECT_EQ(buffer.view().front(), 'X');
    EXPECT_EQ(buffer.view().back(), 'X');

    buffer.consume(1024 * 1024);
    EXPECT_TRUE(buffer.eof());
}

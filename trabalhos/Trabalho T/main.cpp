#include "gtest/gtest.h"
#include "fs.h"
#include "sha256.h"

#include <fstream>
#include <stdio.h>

void duplicate(std::string fsrc, std::string fdest)
{
    std::ifstream  src(fsrc, std::ios::binary);
    std::ofstream  dst(fdest, std::ios::binary);

    dst << src.rdbuf();
}


TEST(FsTest, init021005){
    initFs("fs2-10-5.bin.solucao", 2, 10, 5);
    ASSERT_EQ(printSha256("fs2-10-5.bin.solucao"),std::string("F7:71:A2:19:63:85:52:25:AF:50:89:31:D7:BD:57:9E:BC:5E:3D:A2:85:4F:FE:41:B8:63:1A:5B:18:3F:0E:85"));
}

TEST(FsTest, init011010){
    initFs("fs1-10-10.bin.solucao", 1, 10, 10);
    ASSERT_EQ(printSha256("fs1-10-10.bin.solucao"),std::string("F4:ED:F3:23:45:16:CA:BF:78:1A:BE:6F:EF:DB:7F:0F:BA:07:F5:88:D7:A5:CD:65:1F:18:A4:81:65:91:E3:F4"));
}

TEST(FsTest, case3){
    initFs("fs4-32-16.bin.solucao", 4, 32, 16);
    ASSERT_EQ(printSha256("fs4-32-16.bin.solucao"),std::string("A2:71:21:00:D1:4C:10:94:C9:A0:0A:BD:03:E7:25:38:EA:3E:04:07:57:E4:02:87:5F:7D:1F:B7:35:6D:FE:E4"));
}

TEST(FsTest, case4){
    
    duplicate("fs-case4.bin", "fs-case4.bin.solucao");
    
    addFile("fs-case4.bin.solucao", "/teste.txt", "abc");
    ASSERT_EQ(printSha256("fs-case4.bin.solucao"),std::string("AA:29:B7:CF:09:B6:32:0E:6B:20:51:ED:FD:8E:40:FB:B0:A8:71:FA:8A:22:0A:06:F4:E1:E4:69:0A:C6:B2:77"));
}

TEST(FsTest, case5){
    duplicate("fs-case5.bin", "fs-case5.bin.solucao");
    
    addDir("fs-case5.bin.solucao", "/dec7556");
    ASSERT_EQ(printSha256("fs-case5.bin.solucao"),std::string("0B:BB:60:5C:52:BC:0D:4F:5C:2C:B8:AA:2D:F5:F6:43:7A:EC:02:80:72:F2:D7:C3:7B:91:A6:FE:9E:4C:B6:44"));
    
}

TEST(FsTest, case6){
    duplicate("fs-case6.bin", "fs-case6.bin.solucao");
    
    addFile("fs-case6.bin.solucao", "/dec7556/t2.txt", "fghi");
    ASSERT_EQ(printSha256("fs-case6.bin.solucao"),std::string("C5:D5:15:D8:2F:09:15:49:D9:A2:B5:58:36:E7:DC:28:E5:C4:14:02:1D:03:0E:A8:4E:40:EE:76:BF:05:F0:C6"));
}

TEST(FsTest, case7){
    duplicate("fs-case7.bin", "fs-case7.bin.solucao");

    remove("fs-case7.bin.solucao", "/dec7556/t2.txt");
    ASSERT_EQ(printSha256("fs-case7.bin.solucao"),std::string("67:0C:FE:30:78:13:BE:83:11:47:66:10:19:D2:B8:8F:39:B3:B1:F7:A2:E6:E1:ED:49:ED:1F:11:84:02:B2:B7"));
}

TEST(FsTest, case8){
    duplicate("fs-case8.bin", "fs-case8.bin.solucao");

    remove("fs-case8.bin.solucao", "/dec7556");
    ASSERT_EQ(printSha256("fs-case8.bin.solucao"),std::string("52:EC:46:36:8C:04:DE:F2:75:87:BE:9C:2F:CE:40:39:1A:82:02:05:6A:5D:31:0E:5D:E4:A6:64:94:94:9B:1A"));
}

TEST(FsTest, case9){
    duplicate("fs-case9.bin", "fs-case9.bin.solucao");
    
    move("fs-case9.bin.solucao", "/dec7556/t2.txt", "/t2.txt");
    ASSERT_EQ(printSha256("fs-case9.bin.solucao"),std::string("48:D0:98:B2:5F:BF:D8:4B:A6:37:1F:9A:13:8F:C0:D2:2B:6E:21:39:AB:67:15:7F:DF:AE:3E:23:6D:85:49:04"));
    }

TEST(FsTest, case10){
    duplicate("fs-case10.bin", "fs-case10.bin.solucao");
    
    move("fs-case10.bin.solucao", "/teste.txt", "/dec7556/teste.txt");
    ASSERT_EQ(printSha256("fs-case10.bin.solucao"),std::string("36:EB:18:B6:6F:9C:1E:20:B1:3A:86:81:A7:9D:0B:2E:A4:B8:A1:8E:92:B1:FB:B3:70:15:E8:9E:48:47:FC:53"));
    }

TEST(FsTest, case11){
    duplicate("fs-case11.bin", "fs-case11.bin.solucao");
    
    remove("fs-case11.bin.solucao", "/a.txt");
    ASSERT_EQ(printSha256("fs-case11.bin.solucao"),std::string("06:1C:4A:DC:A4:3C:FF:FC:B9:11:A4:A2:95:02:7B:0D:7F:6E:ED:54:B2:23:65:0B:78:70:C8:CD:59:72:72:64"));
    }

TEST(FsTest, case12){
    duplicate("fs-case12.bin", "fs-case12.bin.solucao");
    
    move("fs-case12.bin.solucao", "/b.txt", "/a.txt");
    ASSERT_EQ(printSha256("fs-case12.bin.solucao"),std::string("BC:2B:05:C8:8B:DF:02:41:3B:E3:86:8E:4C:CC:C1:FF:63:87:F9:A5:24:15:16:49:83:88:F0:75:18:D1:1B:BE"));
    }

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
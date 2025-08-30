#include <atomic>
#include <cstdlib>
#include <gtest/gtest.h>
#include <filesystem>
#include <iterator>
#include <ctime>
#include <cstdint>
#include <fstream>
#include <format>
#include <string>
#include <random>
#include <optional>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <iostream>

#include "filesystem.hpp"

std::filesystem::path image_dir; // A temporary dir for image generation
std::filesystem::path log_dir;   // A directory for failed output
std::filesystem::path src_dir;   // Test source directory
std::optional<uint8_t> forced_image_size; // [1-6] force the size of the generated image (default = random)

class GlobalTestEnv : public ::testing::Environment {
public:
    std::filesystem::path get_absolute_path_from_env(const char* env_name)
    {
	const char* env = std::getenv(env_name);
	if (!env) {
	    const std::string msg = "Define " + std::string(env_name) + " before running the test suite.";
	    throw std::runtime_error(msg);
	}

	return std::filesystem::absolute(env);
    }

    std::optional<uint8_t> get_forced_image_size()
    {
      const char* env = std::getenv("TEST_IMAGE_SIZE");
      if (!env) return std::nullopt;

      int image_size = std::atoi(env);

      if (!(image_size >= 1 && image_size <= 6))
        throw std::runtime_error("TEST_IMAGE_SIZE must be a valid integer between 1 and 6.");

      return (uint8_t)image_size-1;
    }

    void SetUp() override
    {
	image_dir = get_absolute_path_from_env("TEST_IMAGE_DIR");
	log_dir = get_absolute_path_from_env("TEST_LOG_DIR");
	src_dir = get_absolute_path_from_env("TEST_SRC_DIR");
        forced_image_size = get_forced_image_size();
    }
};

::testing::Environment* const global_env = ::testing::AddGlobalTestEnvironment(new GlobalTestEnv());

class ReadTest: public ::testing::Test
{
    void SetUp() override
    {
      std::cout << "Cleaning up..." << std::endl;
      std::filesystem::remove_all(image_dir);

	const std::vector<std::string> sizes = { "very_small", "small", "normal", "large", "very_large" };
	const std::vector<int> weights = { 2, 3, 5, 4, 1 };

	std::random_device rd;
	std::mt19937 gen(rd());

	std::discrete_distribution<> dist(weights.begin(), weights.end());

	std::string fs_size = sizes[dist(gen)];
	if (forced_image_size)
          fs_size = sizes[*forced_image_size];

	const auto cmd = std::string("python3 -u ") + static_cast<std::string>(src_dir) + "/generate_filesystem.py " + static_cast<std::string>(image_dir) + " " + fs_size;

    std::cout << "[Running " << cmd << "]" << std::endl;

      int result = std::system(cmd.c_str());

      if (result != 0) {
	FAIL() << "Failed to generate the test filesystem.";
      }
    }

    void TearDown() override
    {
      if (HasFailure()) {
	if (!std::filesystem::exists(log_dir)) std::filesystem::create_directory(log_dir);
	const auto cmd = std::string("tar -czf") + static_cast<std::string>(log_dir) + "/failed-test-image-" + std::to_string(std::time(nullptr)) + ".tar.gz -C" + static_cast<std::string>(image_dir) + " test.img index.json imagedata";
	std::cout << "Saving the test image... [Press Ctrl+C to cancel]" << std::endl;
	int _ = std::system(cmd.c_str());
      } else {
	std::filesystem::remove_all(image_dir);
      }
    }        
};

#define INPUT_BUFFER_SIZE 1024

class Hasher {
public:
    Hasher();

    void append(std::span<uint8_t> buffer);
    void build(uint8_t output[16]);
    void clear();

private:
    static const uint32_t a = 0x67452301;
    static const uint32_t b = 0xefcdab89;
    static const uint32_t c = 0x98badcfe;
    static const uint32_t d = 0x10325476;

    static const constexpr uint32_t s[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                                           5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                                           4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                                           6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

    static const constexpr uint32_t k[] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
                                           0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
                                           0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
                                           0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
                                           0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
                                           0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
                                           0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
                                           0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
                                           0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
                                           0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
                                           0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
                                           0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
                                           0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
                                           0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                                           0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
                                           0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

    static const constexpr uint8_t padding[] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    inline uint32_t rotateLeft(uint32_t x, uint32_t n) const { return (x << n) | (x >> (32 - n)); }

    inline uint32_t f(uint32_t x, uint32_t y, uint32_t z) const { return ((x & y) | (~x & z)); }
    inline uint32_t g(uint32_t x, uint32_t y, uint32_t z) const { return ((x & z) | (y & ~z)); }
    inline uint32_t h(uint32_t x, uint32_t y, uint32_t z) const { return (x ^ y ^ z); }
    inline uint32_t i(uint32_t x, uint32_t y, uint32_t z) const { return (y ^ (x | ~z)); }

    uint64_t size;
    uint32_t buffer[4];
    uint8_t input[64];

    void step(uint32_t buffer[4], const uint32_t input[16]);
};

Hasher::Hasher() {
    this->clear();
}

void Hasher::clear() {
    this->size = 0;
    
    this->buffer[0] = this->a;
    this->buffer[1] = this->b;
    this->buffer[2] = this->c;
    this->buffer[3] = this->d;
}

void Hasher::step(uint32_t buffer[4], const uint32_t input[16]) {
    uint32_t AA = buffer[0];
    uint32_t BB = buffer[1];
    uint32_t CC = buffer[2];
    uint32_t DD = buffer[3];

    uint32_t E, tmp, j;

    for (int iter=0; iter<64; iter++) {
        switch (iter/16) {
            case 0:
                E = this->f(BB, CC, DD);
                j = iter;
                break;

            case 1:
                E = this->g(BB, CC, DD);
                j = ((iter * 5) + 1) % 16;
                break;

            case 2:
                E = this->h(BB, CC,DD);
                j = ((iter * 3) + 5) % 16;
                break;

            default:
                E = this->i(BB, CC, DD);
                j = (iter * 7) % 16;
                break; 
        }

        tmp = DD;
        DD = CC;
        CC = BB;
        BB = BB + this->rotateLeft(AA + E + this->k[iter] + input[j], this->s[iter]);
        AA = tmp;
    }

    buffer[0] += AA;
    buffer[1] += BB;
    buffer[2] += CC;
    buffer[3] += DD;

}

void Hasher::append(std::span<uint8_t> buffer) {
    uint32_t internal_input[16];
    uint offset = this->size % 64;
    this->size += (uint64_t) buffer.size();

    for (size_t iter=0; iter<buffer.size(); iter++) {
        this->input[offset++] = buffer[iter];

        if (offset % 64 != 0) continue;

        for (int j=0; j<16; j++) {
            internal_input[j] = (uint32_t)(this->input[(j * 4) + 3]) << 24 |
                                (uint32_t)(this->input[(j * 4) + 2]) << 16 |
                                (uint32_t)(this->input[(j * 4) + 1]) <<  8 |
                                (uint32_t)(this->input[(j * 4)]);
        }

        this->step(this->buffer, internal_input);
        offset = 0;
    }
}

void Hasher::build(uint8_t output[16]) {
    uint32_t internal_input[16];
    uint offset = this->size % 64;
    uint padding_length = offset < 56 ? 56 - offset : (56 + 64) - offset;

    this->append(std::span<uint8_t>((uint8_t*)this->padding, padding_length));
    this->size -= padding_length;

    for (int j=0; j<14; j++) {
        internal_input[j] = (uint32_t)(this->input[(j * 4) + 3]) << 24 |
                            (uint32_t)(this->input[(j * 4) + 2]) << 16 |
                            (uint32_t)(this->input[(j * 4) + 1]) <<  8 |
                            (uint32_t)(this->input[(j * 4)]);
    }

    internal_input[14] = (uint32_t)(this->size * 8);
    internal_input[15] = (uint32_t)((this->size * 8) >> 32);

    this->step(this->buffer, internal_input);

    for(unsigned int iter = 0; iter < 4; ++iter) {
        output[(iter * 4) + 0] = (uint8_t)((this->buffer[iter] & 0x000000FF));
        output[(iter * 4) + 1] = (uint8_t)((this->buffer[iter] & 0x0000FF00) >>  8);
        output[(iter * 4) + 2] = (uint8_t)((this->buffer[iter] & 0x00FF0000) >> 16);
        output[(iter * 4) + 3] = (uint8_t)((this->buffer[iter] & 0xFF000000) >> 24);
    }
}

TEST_F(ReadTest, ReadTest)
{
  std::ifstream json_file(static_cast<std::string>(image_dir) + "/index.json");
  const auto data = nlohmann::json::parse(json_file);

  Filesystem fs((static_cast<std::string>(image_dir) + "/test.img").c_str());

  u8* buffer = (u8*)malloc(fs.block_size);
  uint current = 1;
  uint max = data["files"].size();

  for (auto f : data["files"]) {
    const std::string file = f["file"].template get<std::string>();
    const int dir_index = f["root_index"].template get<int>();
    const std::string md5 = f["md5"].template get<std::string>();
    const std::string full_path = "/" + data["directories"][dir_index]["name"].template get<std::string>() + "/" + file;

    std::cout << "[" << current << "/" << max << " (" << (int)(current*100/max) << "%)] " << "Checking the file integrity of " << current << "@" << dir_index << " [" << md5 << "]" << std::endl;

    Inode inode;
    fs.get_inode_from_path(full_path, &inode);


    InodeIterator iter(&fs, inode, buffer);

    Hasher hasher;

    uint8_t output_buf[16];
    
    for (std::span<u8> i : iter) {
      hasher.append(i);
    }

    hasher.build(output_buf);

    std::string rep;
    rep.reserve(32);
    for (int i = 0; i < 16; i++) {
      rep += std::format("{:02x}", output_buf[i]);
    }

    if (rep != md5) {
      FAIL() << "Failed to verify " << full_path << ". The hashes don't match.";
    }

    current++;
  }

  free(buffer);
}

TEST_F(ReadTest, QueryTest)
{
  std::ifstream json_file(static_cast<std::string>(image_dir) + "/index.json");
  const auto data = nlohmann::json::parse(json_file);

  Filesystem fs((static_cast<std::string>(image_dir) + "/test.img").c_str());

  u8* buffer = (u8*)malloc(fs.block_size);
  uint current = 1;
  uint max = data["directories"].size();

  for (auto d : data["directories"]) {
    const int index = current - 1;
    const std::string name  = "/" + d["name"].template get<std::string>();
    const int parent = d["parent_index"].template get<int>();

    std::cout << "[" << current << "/" << max << " (" << (int)(current*100/max) << "%)] " << "Checking the directory query of " << current << std::endl;

    std::unordered_set<std::string> expected;
    std::unordered_set<std::string> got;

    for (auto f : data["files"]) {
	const std::string name = f["file"].template get<std::string>();
	const int dir_index = f["root_index"].template get<int>();

	if (dir_index == index) {
	  expected.insert(name);
        }
    }

    for (auto f : data["directories"]) {
	const std::string name = f["name"].template get<std::string>();
	const int dir_index = f["parent_index"].template get<int>();

	if (dir_index == index) {
          size_t pos = name.rfind('/');
	  std::string last = (pos == std::string::npos) ? name : name.substr(pos + 1);
	  expected.insert(last);
        }
    }

    Inode inode;
    fs.get_inode_from_path(name, &inode);

    u8* buffer = reinterpret_cast<u8*>(malloc(fs.block_size));

    DirInodeIterator dir_iter(&fs, inode, buffer);

    for (DirectoryEntry* entry : dir_iter) {
      if (entry->name(&fs) == "." || entry->name(&fs) == ".." || entry->name(&fs) == "lost+found") continue;
      got.insert(std::string{entry->name(&fs)});
    }

    if (expected != got) {
      FAIL() << "Failed to verify " << name << ". The directory entries don't match";
    }

    current++;
  }

  free(buffer);
}

#pragma once
#include "aegis/kv/file_system.h"
#include <fstream>
#include <filesystem>

namespace aegis::kv {

class RealWritableFile : public WritableFile {
public:
    explicit RealWritableFile(const std::string& path)
        : stream_(path, std::ios::binary | std::ios::app) {}

    bool append(const std::string& data) override {
        stream_.write(data.data(), static_cast<std::streamsize>(data.size()));
        return stream_.good();
    }

    bool sync() override {
        stream_.flush();
        return stream_.good();
    }

    bool close() override {
        stream_.close();
        return true;
    }

private:
    std::ofstream stream_;
};

class RealReadableFile : public ReadableFile {
public:
    explicit RealReadableFile(const std::string& path)
        : path_(path), stream_(path, std::ios::binary) {}

    bool read(uint64_t offset, uint64_t length, std::string& out) override {
        stream_.seekg(static_cast<std::streamoff>(offset));
        out.resize(length);
        stream_.read(out.data(), static_cast<std::streamsize>(length));
        auto got = static_cast<uint64_t>(stream_.gcount());
        out.resize(got);
        return got == length;
    }

    uint64_t size() override {
        return std::filesystem::file_size(path_);
    }

private:
    std::string path_;
    std::ifstream stream_;
};

class RealFileSystem : public FileSystem {
public:
    std::unique_ptr<WritableFile> open_writable(const std::string& path) override {
        return std::make_unique<RealWritableFile>(path);
    }

    std::unique_ptr<ReadableFile> open_readable(const std::string& path) override {
        return std::make_unique<RealReadableFile>(path);
    }

    bool rename(const std::string& from, const std::string& to) override {
        std::error_code ec;
        std::filesystem::rename(from, to, ec);
        return !ec;
    }

    bool remove(const std::string& path) override {
        std::error_code ec;
        return std::filesystem::remove(path, ec);
    }

    std::vector<std::string> list(const std::string& dir) override {
        std::vector<std::string> out;
        for (auto& entry : std::filesystem::directory_iterator(dir)) {
            out.push_back(entry.path().filename().string());
        }
        return out;
    }
};

}  // namespace aegis::kv
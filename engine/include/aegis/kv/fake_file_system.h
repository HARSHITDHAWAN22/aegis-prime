#pragma once
#include "aegis/kv/file_system.h"
#include <map>
#include <random>

namespace aegis::kv {

// In-memory fake used by tests. Lets a test say "crash after the Nth
// write" or "tear this write in half", so recovery code can actually
// be checked instead of assumed correct.
class FakeFileSystem : public FileSystem {
public:
    void set_crash_after_writes(int n) { crash_after_ = n; }
    void set_tear_next_write(bool on) { tear_next_ = on; }

    // What's actually durable right now, i.e. survives a simulated crash.
    std::string synced_content(const std::string& path) {
        return synced_[path];
    }

    std::unique_ptr<WritableFile> open_writable(const std::string& path) override;
    std::unique_ptr<ReadableFile> open_readable(const std::string& path) override;

    bool rename(const std::string& from, const std::string& to) override {
        synced_[to] = synced_[from];
        synced_.erase(from);
        return true;
    }
    bool remove(const std::string& path) override {
        return synced_.erase(path) > 0;
    }
    std::vector<std::string> list(const std::string&) override { return {}; }

    // internal — shared with the file objects below
    std::map<std::string, std::string> synced_;      // durable after sync()
    std::map<std::string, std::string> pending_;      // written but not yet sync()-ed
    int crash_after_ = -1;  // -1 = never
    int write_count_ = 0;
    bool tear_next_ = false;
};

class FakeWritableFile : public WritableFile {
public:
    FakeWritableFile(FakeFileSystem* fs, std::string path) : fs_(fs), path_(std::move(path)) {}

    bool append(const std::string& data) override {
        fs_->write_count_++;
        if (fs_->crash_after_ >= 0 && fs_->write_count_ > fs_->crash_after_) {
            return false;  // simulated crash: write never happens
        }
        std::string to_write = data;
        if (fs_->tear_next_) {
            to_write = data.substr(0, data.size() / 2);  // torn write
            fs_->tear_next_ = false;
        }
        fs_->pending_[path_] += to_write;
        return true;
    }

    bool sync() override {
        fs_->synced_[path_] = fs_->pending_[path_];
        return true;
    }

    bool close() override { return true; }

private:
    FakeFileSystem* fs_;
    std::string path_;
};

class FakeReadableFile : public ReadableFile {
public:
    FakeReadableFile(FakeFileSystem* fs, std::string path) : fs_(fs), path_(std::move(path)) {}

    bool read(uint64_t offset, uint64_t length, std::string& out) override {
        auto& content = fs_->synced_[path_];
        if (offset >= content.size()) { out.clear(); return false; }
        out = content.substr(offset, length);
        return out.size() == length;
    }

    uint64_t size() override { return fs_->synced_[path_].size(); }

private:
    FakeFileSystem* fs_;
    std::string path_;
};

inline std::unique_ptr<WritableFile> FakeFileSystem::open_writable(const std::string& path) {
    return std::make_unique<FakeWritableFile>(this, path);
}
inline std::unique_ptr<ReadableFile> FakeFileSystem::open_readable(const std::string& path) {
    return std::make_unique<FakeReadableFile>(this, path);
}

}  // namespace aegis::kv
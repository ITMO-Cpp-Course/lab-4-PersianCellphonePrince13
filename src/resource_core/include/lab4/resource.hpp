#ifndef LAB4_RESOURCE_CORE_HPP
#define LAB4_RESOURCE_CORE_HPP

#include <exception>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

namespace lab4::resource
{

class ResourceError : public std::exception
{
  public:
    explicit ResourceError(const std::string& message);
    const char* what() const noexcept override;

  private:
    std::string message_;
};

class FileHandle
{
  public:
    explicit FileHandle(const std::string& filename);
    ~FileHandle();

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    FileHandle(FileHandle&& other) noexcept;
    FileHandle& operator=(FileHandle&& other) noexcept;

    void write(const std::string& data);
    std::string read();
    bool is_open() const;
    void close();

    const std::string& get_filename() const;

  private:
    std::string filename_;
    std::fstream file_;

    void open_file();
    void check_open() const;
};

class ResourceManager
{
  public:
    ResourceManager() = default;

    std::shared_ptr<FileHandle> acquire(const std::string& filename);
    void release(const std::string& filename);
    void cleanup();
    size_t cache_size() const;
    size_t active_resources() const;

  private:
    std::unordered_map<std::string, std::weak_ptr<FileHandle>> cache_;
};

} // namespace lab4::resource

#endif

#include "lab4/resource.hpp"
#include <sstream>

namespace lab4::resource
{

ResourceError::ResourceError(const std::string& message) : message_(message) {}

const char* ResourceError::what() const noexcept
{
    return message_.c_str();
}

FileHandle::FileHandle(const std::string& filename) : filename_(filename)
{
    open_file();
}

FileHandle::~FileHandle()
{
    if (file_.is_open())
    {
        file_.close();
    }
}

FileHandle::FileHandle(FileHandle&& other) noexcept
    : filename_(std::move(other.filename_)), file_(std::move(other.file_))
{
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept
{
    if (this != &other)
    {
        if (file_.is_open())
        {
            file_.close();
        }
        filename_ = std::move(other.filename_);
        file_ = std::move(other.file_);
    }
    return *this;
}

void FileHandle::open_file()
{
    file_.open(filename_, std::ios::in | std::ios::out | std::ios::app);
    if (!file_.is_open())
    {
        throw ResourceError("Failed to open file: " + filename_);
    }
}

void FileHandle::check_open() const
{
    if (!file_.is_open())
    {
        throw ResourceError("File is not open: " + filename_);
    }
}

void FileHandle::write(const std::string& data)
{
    check_open();
    file_.clear();
    file_.seekp(0, std::ios::end);
    file_ << data;
    if (file_.fail())
    {
        throw ResourceError("Failed to write to file: " + filename_);
    }
    file_.flush();
}

std::string FileHandle::read()
{
    check_open();
    file_.clear();
    file_.seekg(0, std::ios::beg);

    std::stringstream buffer;
    buffer << file_.rdbuf();

    if (file_.fail() && !file_.eof())
    {
        throw ResourceError("Failed to read from file: " + filename_);
    }

    return buffer.str();
}

bool FileHandle::is_open() const
{
    return file_.is_open();
}

void FileHandle::close()
{
    if (file_.is_open())
    {
        file_.close();
    }
}

const std::string& FileHandle::get_filename() const
{
    return filename_;
}

std::shared_ptr<FileHandle> ResourceManager::acquire(const std::string& filename)
{
    auto it = cache_.find(filename);

    if (it != cache_.end())
    {
        if (auto resource = it->second.lock())
        {
            return resource;
        }
        cache_.erase(it);
    }

    auto resource = std::make_shared<FileHandle>(filename);
    cache_[filename] = resource;
    return resource;
}

void ResourceManager::release(const std::string& filename)
{
    cache_.erase(filename);
}

void ResourceManager::cleanup()
{
    for (auto it = cache_.begin(); it != cache_.end();)
    {
        if (it->second.expired())
        {
            it = cache_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

size_t ResourceManager::cache_size() const
{
    return cache_.size();
}

size_t ResourceManager::active_resources() const
{
    size_t count = 0;
    for (const auto& pair : cache_)
    {
        if (!pair.second.expired())
        {
            count++;
        }
    }
    return count;
}

} // namespace lab4::resource
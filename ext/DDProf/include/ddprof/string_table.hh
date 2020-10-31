#ifndef DDPROF_STRING_TABLE_HH
#define DDPROF_STRING_TABLE_HH

#include <cassert>
#include <cstring>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ddprof {
struct interned_string {
    std::size_t hash;
    std::size_t offset;
    std::size_t size;
    char data[1];  // struct hack, ahoy!

    // not copyable nor move-able due to struct hack
    interned_string(const interned_string &) = delete;
    interned_string(interned_string &&) = delete;
    interned_string &operator=(const interned_string &) = delete;
    interned_string &operator=(interned_string &&) = delete;
};

inline bool operator==(interned_string &lhs, interned_string &rhs) { return &lhs == &rhs; }

struct hashed_string {
    std::string_view view;
    std::size_t hash;
};

inline bool operator==(const hashed_string &lhs, const hashed_string &rhs) { return lhs.view == rhs.view; }

class string_table {
    struct hash {
        std::size_t operator()(const hashed_string &hashed) const noexcept;
    };

    std::unordered_map<hashed_string, std::size_t, hash> map;
    std::vector<const char *> vector;

    interned_string &intern_hashed(const hashed_string &);

   public:
    // string tables aren't copyable
    string_table(const string_table &) = delete;
    string_table &operator=(const string_table &) = delete;

    // they are move-able
    string_table(string_table &&) noexcept;
    string_table &operator=(string_table &&) noexcept;

    string_table();

    /* Requires custom dtor due to storing the interned_string.data instead of
     * storing the interned_string in a unique_ptr or something.
     * I'd rather write a destructor than those shenanigans, honestly.
     */
    ~string_table();

    interned_string &intern(std::string_view);
    interned_string &intern(interned_string &);

    [[nodiscard]] std::size_t size() const noexcept;

    interned_string &operator[](std::size_t offset) noexcept;
    const interned_string &operator[](std::size_t offset) const noexcept;

    void swap(string_table &other) noexcept;
};

inline string_table::string_table(string_table &&) noexcept = default;
inline string_table &string_table::operator=(string_table &&) noexcept = default;

inline std::size_t string_table::hash::operator()(const hashed_string &hashed) const noexcept { return hashed.hash; }

inline string_table::string_table() : map{}, vector{} {
    // the empty string is _always_ offset 0
    auto &interned = intern(std::string_view{});
    assert(interned.offset == 0);
}

inline interned_string &string_table::intern(std::string_view view) {
    std::hash<std::string_view> hasher{};
    hashed_string hashed{view, hasher(view)};
    return intern_hashed(hashed);
}

inline interned_string &string_table::intern_hashed(const hashed_string &hashed) {
    constexpr auto data_offset = offsetof(interned_string, data);
    std::size_t size = hashed.view.size();

    auto iter = map.find(hashed);
    if (iter == map.end()) {
        /* We add +1 for the null terminator. It's not required, but makes
         * working with it easier, as debuggers Just Work.
         */
        void *addr = ::operator new(data_offset + size + 1);
        auto *interned = reinterpret_cast<interned_string *>(addr);
        interned->hash = hashed.hash;
        interned->offset = vector.size();
        interned->size = size;
        memcpy(&interned->data[0], hashed.view.data(), size);
        interned->data[size] = '\0';

        vector.push_back(&interned->data[0]);

        hashed_string updated_hash_string{std::string_view{&interned->data[0], interned->size}, hashed.hash};
        map.emplace(updated_hash_string, interned->offset);
        return *interned;
    }

    char *addr = const_cast<char *>(vector[iter->second] - data_offset);
    return *reinterpret_cast<interned_string *>(addr);
}

inline interned_string &string_table::intern(interned_string &interned) {
    hashed_string hashed{std::string_view{&interned.data[0], interned.size}, interned.hash};
    return intern_hashed(hashed);
}

inline string_table::~string_table() {
    decltype(map) tmpmap{};
    tmpmap.swap(map);

    static constexpr std::size_t data_offset = offsetof(interned_string, data);

    for (const char *data : vector) {
        char *addr = const_cast<char *>(data) - data_offset;
        interned_string &interned = *reinterpret_cast<interned_string *>(addr);
        interned.~interned_string();

        /* We add +1 for the null terminator. It's not required, but makes
         * working with it easier, as tools that work with C strings Just Work.
         */
        ::operator delete(&interned, data_offset + interned.size + 1);
    }

    decltype(vector) tmpvector{};
    tmpvector.swap(vector);
}

inline std::size_t string_table::size() const noexcept {
    assert(vector.size() == map.size());
    return vector.size();
}

inline interned_string &string_table::operator[](std::size_t offset) noexcept {
    assert(offset < size());
    char *addr = const_cast<char *>(vector[offset] - offsetof(interned_string, data));
    return *reinterpret_cast<interned_string *>(addr);
}

inline const interned_string &string_table::operator[](std::size_t offset) const noexcept {
    assert(offset < size());
    const char *addr = vector[offset] - offsetof(interned_string, data);
    return *reinterpret_cast<const interned_string *>(addr);
}

inline void string_table::swap(string_table &other) noexcept {
    vector.swap(other.vector);
    map.swap(other.map);
}

}  // namespace ddprof

namespace std {
template <>
class hash<ddprof::interned_string> {
   public:
    std::size_t operator()(const ddprof::interned_string &string) { return string.hash; }
};
}  // namespace std

#endif  // DDPROF_STRING_TABLE_HH

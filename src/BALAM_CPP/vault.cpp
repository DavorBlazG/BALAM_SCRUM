#include "vault.hpp"
#include "console.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace vault {
namespace {
constexpr std::string_view MAGIC = "BLMV1";
constexpr std::string_view PLAIN_HEADER = "BALAM_VAULT\n";
constexpr std::size_t HEADER_SIZE = MAGIC.size() + 4 + crypto::SALT_SIZE + crypto::NONCE_SIZE + crypto::TAG_SIZE;

// Los campos se separan con tabuladores: se escapan \, tab y salto de linea.
std::string escape(std::string_view text)
{
    std::string out;
    for (char c : text) {
        if (c == '\\') out += "\\\\";
        else if (c == '\t') out += "\\t";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else out += c;
    }
    return out;
}
bool unescape(std::string_view text, std::string& out)
{
    out.clear();
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '\\') { out += text[i]; continue; }
        if (++i >= text.size()) return false;
        if (text[i] == '\\') out += '\\';
        else if (text[i] == 't') out += '\t';
        else if (text[i] == 'n') out += '\n';
        else if (text[i] == 'r') out += '\r';
        else return false;
    }
    return true;
}
std::vector<std::string_view> split(std::string_view text, char separator)
{
    std::vector<std::string_view> parts;
    std::size_t start = 0;
    while (true) {
        const auto pos = text.find(separator, start);
        parts.push_back(text.substr(start, pos == std::string_view::npos ? std::string_view::npos : pos - start));
        if (pos == std::string_view::npos) break;
        start = pos + 1;
    }
    return parts;
}
void wipeKey(crypto::Key& key)
{
    volatile std::uint8_t* p = key.data();
    for (std::size_t i = 0; i < key.size(); ++i) p[i] = 0;
}
}

SessionVault::~SessionVault()
{
    for (auto& entry : entries_) console::wipe(entry.password);
    wipeKey(key_);
}

std::string SessionVault::serialize(const std::vector<Entry>& entries)
{
    std::string out(PLAIN_HEADER);
    for (const auto& e : entries)
        out += std::to_string(e.id) + '\t' + escape(e.service) + '\t' + escape(e.username) + '\t' + escape(e.password) + '\n';
    return out;
}

bool SessionVault::deserialize(std::string_view text, std::vector<Entry>& entries)
{
    if (text.substr(0, PLAIN_HEADER.size()) != PLAIN_HEADER) return false;
    std::vector<Entry> loaded;
    for (auto line : split(text.substr(PLAIN_HEADER.size()), '\n')) {
        if (line.empty()) continue;
        const auto fields = split(line, '\t');
        if (fields.size() != 4) return false;
        Entry e;
        try { e.id = std::stoi(std::string(fields[0])); } catch (...) { return false; }
        if (e.id <= 0) return false;
        if (!unescape(fields[1], e.service) || !unescape(fields[2], e.username) || !unescape(fields[3], e.password))
            return false;
        loaded.push_back(std::move(e));
    }
    entries = std::move(loaded);
    return true;
}

OpenResult SessionVault::open(const std::filesystem::path& file, std::string_view master, std::uint32_t iterations)
{
    file_ = file;
    std::error_code ec;
    if (!std::filesystem::exists(file_, ec)) {
        // Boveda nueva: salt nuevo; el archivo se crea en el primer guardado.
        salt_ = crypto::makeSalt();
        iterations_ = iterations;
        key_ = crypto::deriveKey(master, salt_, iterations_);
        entries_.clear();
        nextId_ = 1;
        opened_ = true;
        return OpenResult::Created;
    }

    std::ifstream in(file_, std::ios::binary);
    const std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (!in.good() && !in.eof()) return OpenResult::Damaged;
    if (data.size() < HEADER_SIZE || std::string_view(data).substr(0, MAGIC.size()) != MAGIC)
        return OpenResult::Damaged;

    std::size_t p = MAGIC.size();
    std::uint32_t storedIterations = 0;
    for (int i = 0; i < 4; ++i) storedIterations = (storedIterations << 8) | static_cast<std::uint8_t>(data[p++]);
    if (storedIterations == 0 || storedIterations > 2000000) return OpenResult::Damaged;

    crypto::Salt salt{};
    crypto::Nonce nonce{};
    crypto::Tag tag{};
    std::copy_n(data.begin() + p, salt.size(), salt.begin());   p += salt.size();
    std::copy_n(data.begin() + p, nonce.size(), nonce.begin()); p += nonce.size();
    std::copy_n(data.begin() + p, tag.size(), tag.begin());     p += tag.size();
    const std::vector<std::uint8_t> cipher(data.begin() + p, data.end());

    crypto::Key key = crypto::deriveKey(master, salt, storedIterations);
    std::string plain;
    std::vector<Entry> loaded;
    if (!crypto::decrypt(key, nonce, cipher, tag, plain) || !deserialize(plain, loaded)) {
        console::wipe(plain);
        wipeKey(key);
        return OpenResult::Damaged;
    }
    console::wipe(plain);

    for (auto& e : entries_) console::wipe(e.password);
    entries_ = std::move(loaded);
    nextId_ = 1;
    for (const auto& e : entries_) nextId_ = std::max(nextId_, e.id + 1);
    key_ = key;
    wipeKey(key);
    salt_ = salt;
    iterations_ = storedIterations;
    opened_ = true;
    return OpenResult::Opened;
}

bool SessionVault::save() const
{
    if (!opened_) return false;
    try {
        crypto::Nonce nonce{};
        crypto::randomBytes(nonce.data(), nonce.size());  // nonce nuevo en cada guardado (nunca repetir en GCM)
        crypto::Tag tag{};
        std::string plain = serialize(entries_);
        const auto cipher = crypto::encrypt(key_, nonce, plain, tag);
        console::wipe(plain);

        std::string data(MAGIC);
        for (int i = 3; i >= 0; --i) data += static_cast<char>((iterations_ >> (8 * i)) & 0xFF);
        data.append(salt_.begin(), salt_.end());
        data.append(nonce.begin(), nonce.end());
        data.append(tag.begin(), tag.end());
        data.append(cipher.begin(), cipher.end());

        // Se escribe en un temporal y se reemplaza: si algo falla, la boveda anterior no se pierde.
        if (file_.has_parent_path()) std::filesystem::create_directories(file_.parent_path());
        auto temporary = file_;
        temporary += ".tmp";
        {
            std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
            if (!out) return false;
            out.write(data.data(), static_cast<std::streamsize>(data.size()));
            out.flush();
            if (!out) return false;
        }
#ifdef _WIN32
        return MoveFileExW(temporary.c_str(), file_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
        ::chmod(temporary.c_str(), S_IRUSR | S_IWUSR);
        std::filesystem::rename(temporary, file_);
        return true;
#endif
    } catch (...) {
        return false;
    }
}

int SessionVault::add(std::string service, std::string username, std::string password)
{
    const int id = nextId_++;
    entries_.push_back({id, std::move(service), std::move(username), std::move(password)});
    return id;
}
const std::vector<Entry>& SessionVault::all() const noexcept { return entries_; }
const Entry* SessionVault::find(int id) const noexcept
{
    const auto it = std::find_if(entries_.begin(), entries_.end(), [id](const Entry& e) { return e.id == id; });
    return it == entries_.end() ? nullptr : &*it;
}
bool SessionVault::update(int id, std::string service, std::string username, std::string password)
{
    for (auto& entry : entries_) {
        if (entry.id != id) continue;
        console::wipe(entry.password);
        entry.service = std::move(service);
        entry.username = std::move(username);
        entry.password = std::move(password);
        return true;
    }
    return false;
}
bool SessionVault::erase(int id)
{
    const auto it = std::find_if(entries_.begin(), entries_.end(), [id](const Entry& e) { return e.id == id; });
    if (it == entries_.end()) return false;
    console::wipe(it->password);
    entries_.erase(it);
    return true;
}
std::vector<Entry> SessionVault::searchByService(std::string_view service) const
{
    const auto lower = [](std::string_view text) {
        std::string out(text);
        for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    };
    const std::string needle = lower(service);
    std::vector<Entry> matches;
    for (const auto& entry : entries_) {
        if (lower(entry.service).find(needle) != std::string::npos)
            matches.push_back({entry.id, entry.service, entry.username, ""});
    }
    return matches;
}
}

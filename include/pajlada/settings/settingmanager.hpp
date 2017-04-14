#pragma once

#include "pajlada/settings/jsonwrapper.hpp"

#include <rapidjson/pointer.h>

#include <algorithm>
#include <memory>
#include <vector>

namespace pajlada {
namespace settings {

namespace detail {

template <typename Type>
class SettingData;

}  // namespace detail

class SettingManager
{
public:
    SettingManager();

    // Print current document json data prettily
    void prettyPrintDocument();

    template <typename Type>
    static void
    unregisterSetting(const std::shared_ptr<detail::SettingData<Type>> &setting)
    {
        SettingManager::localUnregister(setting);
    }

    template <typename Type>
    static void
    registerSetting(std::shared_ptr<detail::SettingData<Type>> setting)
    {
        using namespace std;
        const char *path = setting->getPath().c_str();

        // Save initial value
        // We might want to have this as a setting?
        SettingManager::setValue<Type>(path, setting->getValue());

        // Set up a signal which updates the rapidjson document with the new
        // value when the SettingData value is updated

        // file loaded with SettingManager, this callback will also fire. the
        // only bad part about that is that the setValue method is called
        // unnecessarily
        setting->valueChanged.connect([path](const Type &newValue) {
            SettingManager::setValue<Type>(path, newValue);  //
        });

        // Add the shared_ptr to the relevant vector
        // i.e. std::string SettingData is moved to strSettings
        SettingManager::localRegister(std::move(setting));
    }

    static void setPath(const char *filePath);

    // Clear the loaded json settings
    // XXX(pajlada): What should this actually do?
    static void clear();

    // Load from given path and set given path as the "default path" (or load
    // from default path if nullptr is sent)
    static bool load(const char *filePath = nullptr);
    // Load from given path
    static bool loadFrom(const char *filePath);

    // Force a settings save
    // It is recommended to run this every now and then unless your application
    // is crash free
    // Save to given path and set path as the default path (or save from default
    // path if filePath is a nullptr)
    static bool save(const char *filePath = nullptr);
    // Save to given path
    static bool saveAs(const char *filePath);

    template <typename Type>
    static void
    setValue(const char *path, const Type &value)
    {
        rapidjson::Document &d = getDocument();
        rapidjson::Pointer(path).Set(d, value);
    }

    template <>
    static void
    setValue<std::string>(const char *path, const std::string &value)
    {
        rapidjson::Document &d = getDocument();
        rapidjson::Pointer(path).Set(d, value.c_str());
    }

    template <>
    static void
    setValue<Object>(const char *path, const Object &)
    {
        rapidjson::Document &d = getDocument();
        rapidjson::Pointer(path).Create(d);
    }

    template <>
    static void
    setValue<Array>(const char *path, const Array &)
    {
        rapidjson::Document &d = getDocument();
        rapidjson::Pointer(path).Create(d);
    }

    template <typename Type>
    bool
    loadSetting(std::shared_ptr<detail::SettingData<Type>> setting)
    {
        // A setting should always have a path
        assert(!setting->getPath().empty());

        return this->loadSettingFromPath(setting);
    }

    template <typename Type>
    bool
    loadSettingFromPath(std::shared_ptr<detail::SettingData<Type>> &setting)
    {
        const char *path = setting->getPath().c_str();
        auto value = rapidjson::Pointer(path).Get(this->document);
        if (value == nullptr) {
            return false;
        }

        this->setSetting(setting, *value);

        return true;
    }

    template <typename Type>
    bool setSetting(std::shared_ptr<detail::SettingData<Type>> setting,
                    const rapidjson::Value &value);

    enum class SaveMethod : uint64_t {
        SaveOnExitFlag = (1ull << 1ull),
        SaveOnSettingChangeFlag = (1ull << 2ull),

        // Force user to manually call SettingsManager::save() to save
        SaveManually = 0,
        SaveAllTheTime = SaveOnExitFlag | SaveOnSettingChangeFlag,
    } saveMethod = SaveMethod::SaveAllTheTime;

private:
    // Returns true if the given save method is activated
    inline bool
    checkSaveMethodFlag(SettingManager::SaveMethod testSaveMethod) const
    {
        return (static_cast<uint64_t>(this->saveMethod) &
                static_cast<uint64_t>(testSaveMethod)) != 0;
    }

    static SettingManager *
    manager()
    {
        static SettingManager *m = new SettingManager;

        return m;
    }

    ~SettingManager();

    rapidjson::Document document;
    std::string filePath = "settings.json";

    std::vector<std::shared_ptr<detail::SettingData<int>>> intSettings;
    std::vector<std::shared_ptr<detail::SettingData<bool>>> boolSettings;
    std::vector<std::shared_ptr<detail::SettingData<std::string>>> strSettings;
    std::vector<std::shared_ptr<detail::SettingData<double>>> doubleSettings;
    std::vector<std::shared_ptr<detail::SettingData<float>>> floatSettings;
    std::vector<std::shared_ptr<detail::SettingData<Object>>> objectSettings;
    std::vector<std::shared_ptr<detail::SettingData<Array>>> arraySettings;

    static rapidjson::Document &getDocument();

    template <class Vector, typename Type>
    static void
    removeSettingFrom(Vector &vec,
                      const std::shared_ptr<detail::SettingData<Type>> &setting)
    {
        vec.erase(std::remove_if(std::begin(vec), std::end(vec),
                                 [setting](const auto &item) {
                                     return setting->getConnectionID() ==
                                            item->getConnectionID();
                                 }),
                  std::end(vec));
    }

    template <typename Type>
    static void
    localRegister(std::shared_ptr<detail::SettingData<Type>> setting)
    {
        static_assert(false, "Unimplemented localRegister for setting type");
    }

    template <>
    static void
    localRegister<Array>(std::shared_ptr<detail::SettingData<Array>> setting)
    {
        manager()->arraySettings.push_back(setting);
    }

    template <>
    static void
    localRegister<Object>(std::shared_ptr<detail::SettingData<Object>> setting)
    {
        manager()->objectSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<bool>(std::shared_ptr<detail::SettingData<bool>> setting)
    {
        manager()->boolSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<int>(std::shared_ptr<detail::SettingData<int>> setting)
    {
        manager()->intSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<std::string>(
        std::shared_ptr<detail::SettingData<std::string>> setting)
    {
        manager()->strSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<float>(std::shared_ptr<detail::SettingData<float>> setting)
    {
        manager()->floatSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<double>(std::shared_ptr<detail::SettingData<double>> setting)
    {
        manager()->doubleSettings.push_back(setting);
    }

    template <typename Type>
    static void
    localUnregister(const std::shared_ptr<detail::SettingData<Type>> &setting)
    {
        static_assert(false, "Unimplemented localUnregister for setting type");
        static bool const value = Type::value;
    }

    template <>
    static void
    localUnregister<Array>(
        const std::shared_ptr<detail::SettingData<Array>> &setting)
    {
        SettingManager::removeSettingFrom(manager()->arraySettings, setting);
    }

    template <>
    static void
    localUnregister<Object>(
        const std::shared_ptr<detail::SettingData<Object>> &setting)
    {
        SettingManager::removeSettingFrom(manager()->objectSettings, setting);
    }

    template <>
    static void
    localUnregister<bool>(
        const std::shared_ptr<detail::SettingData<bool>> &setting)
    {
        SettingManager::removeSettingFrom(manager()->boolSettings, setting);
    }

    template <>
    static void
    localUnregister<int>(
        const std::shared_ptr<detail::SettingData<int>> &setting)
    {
        SettingManager::removeSettingFrom(manager()->intSettings, setting);
    }

    template <>
    static void
    localUnregister<std::string>(
        const std::shared_ptr<detail::SettingData<std::string>> &setting)
    {
        SettingManager::removeSettingFrom(manager()->strSettings, setting);
    }

    template <>
    static void
    localUnregister<float>(
        const std::shared_ptr<detail::SettingData<float>> &setting)
    {
        SettingManager::removeSettingFrom(manager()->floatSettings, setting);
    }

    template <>
    static void
    localUnregister<double>(
        const std::shared_ptr<detail::SettingData<double>> &setting)
    {
        SettingManager::removeSettingFrom(manager()->doubleSettings, setting);
    }
};

}  // namespace setting
}  // namespace pajlada

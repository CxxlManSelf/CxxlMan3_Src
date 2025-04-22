#include <sysdef.hpp>
#include <dll_loader.hpp>

namespace CXXL
{

#if defined(PLATFORM_NAME)
  #if (PLATFORM_NAME == _WINDOWS_CxxlMan3)
    #include <windows.h>
    typedef HMODULE library_handle;
  #else
    #if (PLATFORM_NAME != _UNKNOWN_CxxlMan3)
      #include <dlfcn.h>
      typedef void *library_handle;
    #else
      #error "Unknown platform"
    #endif
  #endif
#endif

    class DllLoader : public IDllLoader
    {
        library_handle m_handle;

        // 指向自己
        // 為了方便取得自己的 std::shared_ptr
        std::weak_ptr<IDllLoader> m_DllLoader_ptr;

        std::shared_ptr<IDllLoader> cxxlFASTCALL getDllLoader() override
        {
            return m_DllLoader_ptr.lock();
        }

        virtual void *cxxlFASTCALL getProcAddress(const cxxlSTDSTRING &procName) override
        {
#if (PLATFORM_NAME == _WINDOWS_CxxlMan3)
            return (void *)GetProcAddress(m_handle, (const char *)procName.c_str());
#else
            return dlsym(m_handle, procName.c_str());
#endif
        }

    public:
        // Constructor
        DllLoader(const cxxlSTDSTRING &dllPath)
        {
#if (PLATFORM_NAME == _WINDOWS_CxxlMan3)
            m_handle = LoadLibraryA((const char *)dllPath.c_str());
#else
            m_handle = dlopen(dllPath.c_str(), RTLD_LAZY);
#endif
        }

        // Destructor
        ~DllLoader()
        {
#if (PLATFORM_NAME == _WINDOWS_CxxlMan3)
            if (m_handle != nullptr)
                FreeLibrary(m_handle);
#else
            if (m_handle != nullptr)
                dlclose(m_handle);
#endif
        }

        bool cxxlFASTCALL isValid() const
        {
            return m_handle != nullptr;
        }

        void cxxlFASTCALL setDllLoader(const std::shared_ptr<IDllLoader> &DllLoader_ptr)
        {
            m_DllLoader_ptr = DllLoader_ptr;
        }
    };

    std::shared_ptr<IDllLoader> IDllLoader::create(const std::u8string &dllPath)
    {
        DllLoader *pDllLoader(new DllLoader(dllPath));
        if (!pDllLoader->isValid())
        {
            delete pDllLoader;
            return std::shared_ptr<IDllLoader>(nullptr);
        }
        else
        {
            std::shared_ptr<IDllLoader> DllLoader_ptr = std::shared_ptr<IDllLoader>(pDllLoader);
            pDllLoader->setDllLoader(DllLoader_ptr);
            return DllLoader_ptr;
        }
    }
}
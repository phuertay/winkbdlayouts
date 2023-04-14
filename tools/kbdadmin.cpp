//---------------------------------------------------------------------------
//
// Windows Keyboards Layouts (WKL)
// Copyright (c) 2023, Thierry Lelegard
// BSD-2-Clause license, see the LICENSE file.
//
// Utility to manage or install keyboard layout DLL's.
//
//---------------------------------------------------------------------------

#include "options.h"
#include "registry.h"
#include "strutils.h"
#include "winutils.h"
#include "grid.h"
#include "kbdrc.h"

// Configure the terminal console on init, restore on exit.
ConsoleState state;


//----------------------------------------------------------------------------
// Command line options.
//----------------------------------------------------------------------------

class AdminOptions : public Options
{
public:
    // Constructor.
    AdminOptions(int argc, wchar_t* argv[]);

    // Command line options.
    WString       output;
    WStringVector dll_install;
    bool          remove_wkl;
    bool          list_keyboards;
    bool          show_user;
    bool          search_active;
};

AdminOptions::AdminOptions(int argc, wchar_t* argv[]) :
    Options(argc, argv,
        L"[options]\n"
        L"\n"
        L"Options:\n"
        L"\n"
        L"  -i dll-or-directory : install specified keyboard DLL\n"
        L"  -o file : output file name, default is standard output\n"
        L"  -h  : display this help text\n"
        L"  -l  : list installed keyboards\n"
        L"  -p  : prompt user on end of execution\n"
        L"  -np : ignore -p, don't prompt\n"
        L"  -r  : remove all installed keyboard DLL's from this project\n"
        L"  -s  : search active keyboard layout DLL's in all processes\n"
        L"  -u  : display user setup"),
    output(),
    dll_install(),
    remove_wkl(false),
    list_keyboards(false),
    show_user(false),
    search_active(false)
{
    bool dont_prompt = false;

    // Parse arguments.
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == L"--help" || args[i] == L"-h") {
            usage();
        }
        else if (args[i] == L"-l") {
            list_keyboards = true;
        }
        else if (args[i] == L"-s") {
            search_active = true;
        }
        else if (args[i] == L"-u") {
            show_user = true;
        }
        else if (args[i] == L"-r") {
            remove_wkl = true;
        }
        else if (args[i] == L"-p") {
            setPromptOnExit(!dont_prompt);
        }
        else if (args[i] == L"-np") {
            dont_prompt = true;
            setPromptOnExit(false);
        }
        else if (args[i] == L"-o" && i + 1 < args.size()) {
            output = args[++i];
        }
        else if (args[i] == L"-i" && i + 1 < args.size()) {
            dll_install.push_back(args[++i]);
        }
        else {
            fatal("invalid option '" + args[i] + "', try --help");
        }
    }

    // Default action if nothing is specified.
    if (!list_keyboards && !show_user && !search_active && !remove_wkl && dll_install.empty()) {
        // If the exe is named "setup.exe", default to "-i same-directory-as-exe".
        const WString exe(GetCurrentProgram());
        if (ToLower(FileName(exe)) == L"setup.exe") {
            dll_install.push_back(DirName(exe));
        }
        else {
            fatal(L"no option specified, try --help");
        }
    }
}


//---------------------------------------------------------------------------
// List available keyboards
//---------------------------------------------------------------------------

void ListKeyboards(AdminOptions& opt)
{
    // Enumerate keyboard layouts in registry.
    Registry reg(opt);
    WStringList layout_ids;
    if (!reg.getSubKeys(REGISTRY_LAYOUT_KEY, layout_ids)) {
        return;
    }

    Grid grid(L"", L"  ");
    grid.addLine({L"Id", L"Lang", L"File", L"Description"});
    grid.addUnderlines();

    for (const auto& id : layout_ids) {
        const WString key(REGISTRY_LAYOUT_KEY "\\" + id);
        const WString file(reg.getValue(key, REGISTRY_LAYOUT_FILE));
        WString text(reg.getValue(key, REGISTRY_LAYOUT_DISPLAY, true));
        if (text.empty()) {
            text = reg.getValue(key, REGISTRY_LAYOUT_TEXT);
        }
        WString lang(FileBaseName(ToLower(file)));
        if (StartsWith(lang, L"kbd")) {
            lang.erase(0, 3);
        }
        else {
            lang.clear();
        }
        grid.addLine({id, lang, file, text});
    }
    opt.out() << std::endl;
    grid.print(opt.out());
}


//---------------------------------------------------------------------------
// Display user setup.
//---------------------------------------------------------------------------

void DisplayUserSetup(AdminOptions& opt)
{
    // Enumerate user's preloads in registry.
    Registry reg(opt);
    WStringList names;
    if (!reg.getValueNames(REGISTRY_USER_PRELOAD_KEY, names)) {
        return;
    }

    Grid grid(L"", L"  ");
    for (const auto& n : names) {
        const WString id(reg.getValue(REGISTRY_USER_PRELOAD_KEY, n));
        grid.addLine({n + L":", id});
        const WString idkey(REGISTRY_LAYOUT_KEY "\\" + id);
        if (reg.keyExists(idkey)) {
            grid.addColumn(reg.getValue(idkey, REGISTRY_LAYOUT_TEXT) + L" (" + reg.getValue(idkey, REGISTRY_LAYOUT_FILE) + L")");
        }
    }

    opt.out() << std::endl
              << "Preload" << std::endl
              << "-------" << std::endl;
    grid.print(opt.out());

    // Enumertate user's substitutions.
    if (!reg.getValueNames(REGISTRY_USER_SUBSTS_KEY, names)) {
        return;
    }

    grid.clear();
    for (const auto& n : names) {
        const WString id(reg.getValue(REGISTRY_USER_SUBSTS_KEY, n));
        grid.addLine({n, L"->", id});
        const WString idkey(REGISTRY_LAYOUT_KEY "\\" + id);
        if (reg.keyExists(idkey)) {
            grid.addColumn(reg.getValue(idkey, REGISTRY_LAYOUT_TEXT) + L" (" + reg.getValue(idkey, REGISTRY_LAYOUT_FILE) + L")");
        }
    }

    opt.out() << std::endl
              << "Substitutes" << std::endl
              << "-----------" << std::endl;
    grid.print(opt.out());
}


//---------------------------------------------------------------------------
// Search active keyboard DLL's in all processes.
//---------------------------------------------------------------------------

void SearchActiveKeyboards(AdminOptions& opt)
{
    // Enumerate keyboard layouts in registry.
    Registry reg(opt);
    WStringList layout_ids;
    if (!reg.getSubKeys(REGISTRY_LAYOUT_KEY, layout_ids)) {
        return;
    }

    // Get all known keybord DLL's.
    WStringSet known_dlls;
    for (const auto& id : layout_ids) {
        const WString key(REGISTRY_LAYOUT_KEY "\\" + id);
        const WString file(reg.getValue(key, REGISTRY_LAYOUT_FILE));
        if (!file.empty()) {
            known_dlls.insert(ToLower(FileName(file)));
        }
    }

    // Get all process ids in the system.
    std::vector<DWORD> pids(4096);
    DWORD insize = DWORD(pids.size() * sizeof(DWORD));
    DWORD retsize = 0;
    if (!EnumProcesses(&pids[0], insize, &retsize)) {
        const DWORD err = GetLastError();
        opt.fatal("EnumProcesses error: " + ErrorText(err));
    }
    pids.resize(retsize / sizeof(DWORD));

    // Loop on all processes.
    opt.out() << std::endl;
    size_t error_count = 0;
    for (auto pid : pids) {

        // Open access to process.
        if (pid == 0) {
            continue; // why do we get pid zero ?
        }
        HANDLE hproc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
        if (hproc == NULL) {
            const DWORD err = GetLastError();
            error_count++;
            continue;
        }

        // Enumerate all DLL's in this process.
        std::vector<HMODULE> mods(4096);
        insize = DWORD(mods.size() * sizeof(HMODULE));
        retsize = 0;
        if (!EnumProcessModules(hproc, &mods[0], insize, &retsize)) {
            const DWORD err = GetLastError();
            error_count++;
            CloseHandle(hproc);
            continue;
        }
        mods.resize(retsize / sizeof(HMODULE));

        // Get the module names and display potential keyboards DLL's.
        for (auto hmod : mods) {
            const WString file(ModuleFileName(hproc, hmod));
            if (known_dlls.find(ToLower(FileName(file))) != known_dlls.end()) {
                opt.out() << Format(L"Process 0x%08X ", pid) << FileName(ModuleFileName(hproc, nullptr)) << " uses " << file << std::endl;
            }
        }
        CloseHandle(hproc);
    }
    opt.out() << "Found " << pids.size() << " processes, " << error_count << " cannot be accessed" << std::endl;
}


//---------------------------------------------------------------------------
// Install one keyboard DLL's.
//---------------------------------------------------------------------------

void InstallOneKeyboard(AdminOptions& opt, const WString& dll)
{
    // Get all expected resource strings from a WKL DLL.
    HMODULE hmod = LoadLibraryExW(dll.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
    if (hmod == nullptr) {
        const DWORD err = GetLastError();
        opt.error("error opening " + dll + ": " + ErrorText(err));
        return;
    }
    const WString text(GetResourceString(hmod, WKL_RES_TEXT));
    const WString provider(GetResourceString(hmod, WKL_RES_PROVIDER));
    WString lang(ToLower(GetResourceString(hmod, WKL_RES_LANG)));
    FreeLibrary(hmod);

    // A WKL DLL is expected to have "WKL" as provider resource string.
    if (provider != L"WKL") {
        return;
    }

    opt.out() << "==== Installing " << dll << " (" << text << ")" << std::endl;

    // Adjust language to 4 hexa digits.
    if (lang.length() != 4) {
        WString new_lang("0000" + lang);
        new_lang = new_lang.substr(new_lang.length() - 4);
        opt.out() << "warning: invalid base language \"" << lang << "\", using \"" << new_lang << "\"" << std::endl;
        lang = new_lang;
    }

    // Enumerate keyboard layouts in registry.
    Registry reg(opt);
    WStringList layout_ids;
    if (!reg.getSubKeys(REGISTRY_LAYOUT_KEY, layout_ids)) {
        return;
    }

    // Build a list of ids with matching base language.
    WStringSet matching_ids;
    WString final_id;
    const WString file_name(ToLower(FileName(dll)));

    for (const auto& id : layout_ids) {
        // Check if the id matches the base language of the new keyboard.
        if (EndsWith(ToLower(id), lang)) {
            const WString key(REGISTRY_LAYOUT_KEY "\\" + id);
            if (ToLower(reg.getValue(key, REGISTRY_LAYOUT_FILE)) == file_name) {
                opt.out() << file_name << " already registered, replacing it" << std::endl;
                final_id = id;
                break;
            }
            matching_ids.insert(id);
        }
    }

    // Allocate a layout id if not already registered.
    if (final_id.empty()) {
        // Find an unused id with base language. Depends on the size of the base language.
        // Example: if lang = "040c", try "a001040c", "a003040c", "a003040c", 
        for (int up = 0xA000; up < 0xB000; ++up) {
            final_id = Format(L"%04x%s", up, lang.c_str());
            if (matching_ids.find(final_id) == matching_ids.end()) {
                break; // unused id
            }
        }
    }

    // Copy DLL file first.
    const WString destination(GetEnv(L"SystemRoot", L"C:\\Windows") + L"\\System32\\" + file_name);
    opt.out() << "registering " << final_id << ": " << destination << " (" << text << ")" << std::endl;
    if (!CopyFileW(dll.c_str(), destination.c_str(), false)) {
        const DWORD err = GetLastError();
        opt.error("error creating " + destination + ": " + ErrorText(err));
        return;
    }

    // Then register it in the registry.
    const WString key(REGISTRY_LAYOUT_KEY "\\" + final_id);
    if (!reg.keyExists(key) && !reg.createKey(key)) {
        return;
    }
    reg.setValue(key, REGISTRY_LAYOUT_FILE, file_name);
    reg.setValue(key, REGISTRY_LAYOUT_TEXT, text);
    reg.setValue(key, REGISTRY_LAYOUT_PROVIDER, provider);
    reg.setValue(key, REGISTRY_LAYOUT_DISPLAY, Format(L"@%%SystemRoot%%\\system32\\%s,-%d", file_name.c_str(), WKL_RES_TEXT), true);
}


//---------------------------------------------------------------------------
// Install all specified keyboard DLL's.
//---------------------------------------------------------------------------

void InstallKeyboards(AdminOptions& opt)
{
    for (const auto& path : opt.dll_install) {
        if (IsDirectory(path)) {
            WStringList files;
            SearchFiles(files, path, L"*.dll");
            for (const auto& file : files) {
                InstallOneKeyboard(opt, path + L"\\" + file);
            }
        }
        else {
            InstallOneKeyboard(opt, path);
        }
    }
}


//---------------------------------------------------------------------------
// Remove all installed WKL keyboard DLL's.
//---------------------------------------------------------------------------

void RemoveKeyboards(AdminOptions& opt)
{
    // Enumerate keyboard layouts in registry.
    Registry reg(opt);
    WStringList layout_ids;
    if (!reg.getSubKeys(REGISTRY_LAYOUT_KEY, layout_ids)) {
        return;
    }

    // Keep track of removed files.
    WStringSet removed_files;
    const WString sysdir(GetEnv(L"SystemRoot", L"C:\\Windows") + L"\\System32\\");

    // Remove all WKL keyboads.
    for (const auto& id : layout_ids) {
        const WString key(REGISTRY_LAYOUT_KEY "\\" + id);
        const WString file(reg.getValue(key, REGISTRY_LAYOUT_FILE));
        const WString path(sysdir + file);
        const WString lowfile(ToLower(file));
        bool unreg = removed_files.find(lowfile) != removed_files.end();
        if (!unreg && GetResourceString(path, WKL_RES_PROVIDER) == L"WKL") {
            opt.out() << "Removing " << path << " (" << id << ")" << std::endl;
            if (DeleteFileW(path.c_str())) {
                removed_files.insert(lowfile);
                unreg = true;
            }
            else {
                const DWORD err = GetLastError();
                opt.error("error deleting " + path + ": " + ErrorText(err));
                unreg = false;
            }
        }
        if (unreg) {
            opt.out() << "Unregistering " << id << std::endl;
            reg.deleteKey(key);
        }
    }
}


//---------------------------------------------------------------------------
// Application entry point.
//---------------------------------------------------------------------------

int wmain(int argc, wchar_t* argv[])
{
    // Parse command line options.
    AdminOptions opt(argc, argv);

    // Need to be admin to install keyboards or explore all processes.
    if ((!opt.dll_install.empty() || opt.remove_wkl || opt.search_active) && !IsAdmin()) {
        std::cout << "Restarting as admin..." << std::endl;
        RestartAsAdmin(opt.args + L"-p", true);
        opt.exit(EXIT_SUCCESS);
    }

    // Now perform all requested operations.
    opt.setOutput(opt.output);
    if (opt.remove_wkl) {
        RemoveKeyboards(opt);
    }
    if (!opt.dll_install.empty()) {
        InstallKeyboards(opt);
    }
    if (opt.list_keyboards) {
        ListKeyboards(opt);
    }
    if (opt.show_user) {
        DisplayUserSetup(opt);
    }
    if (opt.search_active) {
        SearchActiveKeyboards(opt);
    }
    opt.out() << std::endl;
    opt.exit(EXIT_SUCCESS);
}

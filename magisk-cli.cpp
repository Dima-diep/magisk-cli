#include <sqlite3.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>
#include <algorithm>
#include <sys/types.h>
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <filesystem>
#include <zlib.h>
using namespace std;
namespace file = std::filesystem;

extern "C" {
void permission_denied() {
    fprintf(stderr, "magisk-cli: Permission denied!\n");
    exit(1);
}
void key_error() {
    fprintf(stderr, "Error! Use additional key: u, n, or i\n");
    fprintf(stderr, "u - for appuser (u0_a150 like)\n");
    fprintf(stderr, "n - for appname (com.termux like)\n");
    fprintf(stderr, "i - for app UID (10150 like) (it always > 10000)\n");
    exit(1);
}
void boot_help() {
    printf("Manage boot.img\n");
    printf("Usage: magisk-cli -b (options) (/path/to/boot.img)\n");
    printf("Usage: magisk-cli -b -r (/path/to/ramdisk) (options)\n");
    printf("\nOptions:\n");
    printf("\n    -p - Patch stock boot.img\n");
    printf("    -u - Unpack boot.img to kernel, dtb, ramdisk.cpio\n");
    printf("    -uh - Unpack boot.img to kernel, dtb, ramdisk.cpio, header\n");
    printf("    -r (ramdisk.cpio) (commands) - Ramdisk commands\n");
    printf("    -hx (pattern1) (pattern2) (kernel/zImage) - hexpatch kernel\n");
    printf("    -b (oldboot.img) (newboot.img) - rebuild boot.img. oldboot.img need for checking signatures, headers, cmdline and offsets for correct building of newboot.img. Youneed these files: ramdisk.cpio, oldboot.img, kernel, other files if they extracted (usually its 'extra', 'kernel_dtb', 'header')\n");
    printf("\nRamdisk commands:\n");
    printf("    -x - unpack ramdisk.cpio\n");
    printf("    -xg - unpack ramdisk.cpio.gz\n");
    printf("    -xx - unpack ramdisk.cpio.xz\n");
    printf("    -d (file) - delete file from ramdisk.cpio\n");
    printf("    -a (mode) (file) (entry) - add file to ramdisk.cpio\n");
    printf("    -m (mode) (directory) - create directory into ramdisk.cpio\n");
    printf("    -o (directory) - unpack ramdisk to directory\n");
}
void magiskhide_help() {
    printf("Manage MagiskHide\n");
    printf("Usage: magisk-cli -H (options) (app)\n");
    printf("\n    -e - activate MagiskHide\n");
    printf("    -d - disable MagiskHide\n");
    printf("    -a (app) - add app to MagiskHide\n");
    printf("    -r (app) - delete app from MagiskHide\n");
};
void denylist_help() {
    printf("Manage DenyList\n");
    printf("Usage: magisk-cli -z (options) (app)\n");
    printf("\nOptions:\n");
    printf("\n    -e - activate DenyList\n");
    printf("    -d - disable DenyList\n");
    printf("    -a (app) - add app to DenyList\n");
    printf("    -r (app) - delete app from DenyList\n");
};
void module_help() {
    printf("Manage modules\n");
    printf("Usage: magisk-cli -m (options) (module)\n");
    printf("\nOptions:\n");
    printf("    -i (zipfile) - install module\n");
    printf("    -off (module) - disable module\n");
    printf("    -u (module) - uninstall module\n");
    printf("    -l - list modules\n");
};
void root_help() {
    printf("Manage root access\n");
    printf("Usage: magisk-cli -r (options) (app)\n");
    printf("\nOptions:\n");
    printf("\n    -a (uid) (app) - add app\n");
    printf("    -x (uid) (app) - deny root access\n");
    printf("    -e (uid) (app) - enable root access\n");
    printf("    -d (uid) (app) - delete app\n");
    printf("    -le - print list of rooted apps\n");
};
void print_help() {
    printf("Usage: magisk-cli (key) (option)\n");
    printf("\n    -r - Manage root access\n");
    printf("    -m - Manage modules\n");
    printf("    -z - Manage DenyList\n");
    printf("    -H - Manage MagiskHide\n");
    printf("    -b - Manage boot.img/ramdisk\n");
    printf("    -c (ramdisk) (directory) - Create ramdisk from directory\n");
    printf("    -h - Help\n");
};
void sql_failed() {
    fprintf(stderr, "Can't open /data/adb/magisk.db!\n");
};
void nofile() {
    fprintf(stderr, "Can't find file!\n");
    exit(1);
};
void sh_exec(const char* command) {
    try {
        execlp("sh", "sh", "-c", command, NULL);
    } catch (...) {
        permission_denied();
    }
}
int sql_callback_echo(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int a=0; a<argc; a++) {
        printf("%d.%s\n", a, argv[a]);
    }
    return 0;
}
int sql_callback(void *NotUsed, int argc, char **argv, char **azColName) {
    return 0;
}
}

int main(int argc, char* argv[]) {
    if (argc==1) {
        print_help();
    } else if (argc==2) {
        const string arg_first = argv[1];
        if (arg_first=="-r") {
            root_help();
        } else if (arg_first=="-m") {
            module_help();
        } else if (arg_first=="-z") {
            denylist_help();
        } else if (arg_first=="-H") {
            magiskhide_help();
        } else if (arg_first=="-b") {
            boot_help();
        } else {
            print_help();
        }
    } else {
        const string arg_first = argv[1];
        const string arg_second = argv[2];
        if (arg_first=="-r") {
            if (arg_second=="-le") {
                sqlite3 *db = 0;
                if (sqlite3_open("/data/adb/magisk.db", &db)) {
                    sql_failed();
                } else {
                    char *sql_error = 0;
                    const char* sqcmd = "select package_name from policies;";
                    const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback_echo, 0, &sql_error);
                }
                sqlite3_close(db);
            } else if (arg_second=="-a") {
                if (argc < 5) {
                    root_help();
                } else {
                    sqlite3 *db = 0;
                    if (sqlite3_open("/data/adb/magisk.db", &db)) {
                        sql_failed();
                    } else {
                        const string uid = argv[3];
                        const string appname = argv[4];
                        char *sql_error = 0;
                        const char* sqcmd = ("INSERT INTO policies (uid, package_name, policy, until, logging, notification) VALUES (" + uid + ", " + appname + ", 2, 0, 1, 1);").c_str();
                        const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                    }
                    sqlite3_close(db);
                }
            } else if (arg_second=="-x") {
                if (argc < 5) {
                    root_help();
                } else {
                    sqlite3 *db = 0;
                    if (sqlite3_open("/data/adb/magisk.db", &db)) {
                        sql_failed();
                    } else {
                        const string uid = argv[3];
                        const string appname = argv[4];
                        char *sql_error = 0;
                        const char* sqcmd = ("UPDATE policies SET policy=1 WHERE uid = " + uid + " AND package_name = " + appname + ";").c_str();
                        const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                    }
                    sqlite3_close(db);
                }
            } else if (arg_second=="-e") {
                if (argc < 5) {
                    root_help();
                } else {
                    sqlite3 *db = 0;
                    if (sqlite3_open("/data/adb/magisk.db", &db)) {
                        sql_failed();
                    } else {
                        const string uid = argv[3];
                        const string appname = argv[4];
                        char *sql_error = 0;
                        const char* sqcmd = ("UPDATE policies SET policy=2 WHERE uid = " + uid + " AND package_name = " + appname + ";").c_str();
                        const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                    }
                    sqlite3_close(db);
                }
            } else if (arg_second=="-d") {
                if (argc < 5) {
                    root_help();
                } else {
                    sqlite3 *db = 0;
                    if (sqlite3_open("/data/adb/magisk.db", &db)) {
                        sql_failed();
                    } else {
                        const string uid = argv[3];
                        const string appname = argv[4];
                        char *sql_error = 0;
                        const char* sqcmd = ("DELETE FROM policies WHERE uid = " + uid + " AND package_name = " + appname + ";").c_str();
                        const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                    }
                    sqlite3_close(db);
                }
            } else {
                root_help();
            }
        } else if (arg_first=="-m") {
            if (arg_second=="-i") {
                if (argc < 4) {
                    module_help();
                } else {
                    const string zipfile = argv[3];
                    struct stat zipbuf;
                    const int ifext = stat(zipfile.c_str(), &zipbuf);
                    if (ifext==0) {
                        try {
                            execlp("/sbin/magisk", "/sbin/magisk", "--install-module", zipfile.c_str(), NULL);
                        } catch(...) {
                            permission_denied();
                        }
                    }
                    else {
                         fprintf(stderr, "%s: No such file or directory\n", zipfile.c_str());
                         exit(1);
                    }
                }
            } else if (arg_second=="-off") {
                if (argc < 4) {
                    module_help();
                } else {
                    const string module = argv[3];
                    struct stat modulebuf;
                    const int ifext = stat(("/data/adb/modules/" + module).c_str(), &modulebuf);
                    if (ifext==0) {
                        const int disabled = open(("/data/adb/modules/" +  module + "/disable").c_str(), O_CREAT);
                        close(disabled);
                        struct stat disbuf;
                        const int isdisabled = stat(("/data/adb/modules/" + module + "/disabled").c_str(), &disbuf);
                        if (isdisabled==0) {
                            printf("Module %s successfully disabled\n", module.c_str());
                        } else {
                            fprintf(stderr, "Can't disable module: %s\n", module.c_str());
                            exit(1);
                        }
                    } else {
                        fprintf(stderr, "Module %s does not exist!\n", module.c_str());
                        exit(1);
                    }
                }
            } else if (arg_second=="-u") {
                if (argc < 4) {
                    module_help();
                }
                else {
                    const string module = argv[3];
                    struct stat modulebuf;
                    const int ifext = stat(("/data/adb/modules/" + module).c_str(), &modulebuf);
                    if (ifext==0) {
                        try {
                            rmdir(("/data/adb/modules/" + module).c_str());
                            rmdir(("/data/adb/modules_update/" + module).c_str());
                        } catch(...) {
                            permission_denied();
                        }
                    } else {
                        fprintf(stderr, "Module %s does not exist!\n", module.c_str());
                        exit(1);
                    }
                }
            } else if (arg_second=="-l") {
                unsigned int *countm = new unsigned int;
                *countm = 0;
                for (auto a : file::directory_iterator("/data/adb/modules")) {
                    *countm += 1;
                    string fpath = (a.path()).c_str();
                    fpath.replace(fpath.find("/data/adb/modules/"), 18, "");
                    printf("%d.%s\n", *countm, fpath);
                }
                delete countm;
            } else {
                module_help();
            }
        } else if (arg_first=="-z") {
            if (arg_second=="-e") {
                sqlite3 *db = 0;
                if (sqlite3_open("/data/adb/magisk.db", &db)) {
                    sql_failed();
                } else {
                    char *sql_error = 0;
                    const char* sqcmd = "UPDATE settings SET value=1 WHERE key = 'zygisk'";
                    const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                }
                sqlite3_close(db);
            } else if (arg_second=="-d") {
                sqlite3 *db = 0;
                if (sqlite3_open("/data/adb/magisk.db", &db)) {
                    sql_failed();
                } else {
                    char *sql_error = 0;
                    const char* sqcmd = "UPDATE settings SET value=0 WHERE key = 'zygisk'";
                    const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                }
                sqlite3_close(db);
            } else if (arg_second=="-a") {
                if (argc < 4) {
                    denylist_help();
                } else {
                    const string appname = argv[3];
                    struct stat appbuf;
                    const int ifext = stat(("/data/data/" + appname).c_str(), &appbuf);
                    if (ifext==0) {
                        sqlite3 *db = 0;
                        if (sqlite3_open("/data/adb/magisk.db", &db)) {
                            sql_failed();
                        } else {
                            char *sql_error = 0;
                            const char* sqcmd = ("INSERT INTO denylist (package_name, process) VALUES (" + appname + ", " + appname + ");").c_str();
                            const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                        }
                        sqlite3_close(db);
                    } else {
                        fprintf(stderr, "Error: app %s not found!\n", appname.c_str());
                        exit(1);
                    }
                }
            } else if (arg_second=="-r") {
                if (argc < 4) {
                    denylist_help();
                } else {
                    const string appname = argv[3];
                    struct stat appbuf;
                    const int ifext = stat(("/data/data/" + appname).c_str(), &appbuf);
                    if (ifext==0) {
                        sqlite3 *db = 0;
                        if (sqlite3_open("/data/adb/magisk.db", &db)) {
                            sql_failed();
                        } else {
                            char *sql_error = 0;
                            const char* sqcmd = ("DELETE FROM denylist WHERE package_name = " + appname + ";").c_str();
                            const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                        }
                        sqlite3_close(db);
                    } else {
                        fprintf(stderr, "Error: app %s not found!\n", appname.c_str());
                        exit(1);
                    }
                }
            } else {
                denylist_help();
            }
        } else if (arg_first=="-H") {
            if (arg_second=="-e") {
                sqlite3 *db = 0;
                if (sqlite3_open("/data/adb/magisk.db", &db)) {
                    sql_failed();
                } else {
                    char *sql_error = 0;
                    const char* sqcmd = "UPDATE settings SET value=1 WHERE key = 'magiskhide';";
                    const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                }
                sqlite3_close(db);
            } else if (arg_second=="-d") {
                sqlite3 *db = 0;
                if (sqlite3_open("/data/adb/magisk.db", &db)) {
                    sql_failed();
                } else {
                    char *sql_error = 0;
                    const char* sqcmd = "UPDATE settings SET value=1 WHERE key = 'magiskhide';";
                    const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                }
                sqlite3_close(db);
            } else if (arg_second=="-a") {
                if (argc < 4) {
                    magiskhide_help();
                } else {
                    const string appname = argv[3];
                    struct stat appbuf;
                    const int ifext = stat(("/data/data/" + appname).c_str(), &appbuf);
                    if (ifext==0) {
                        sqlite3 *db = 0;
                        if (sqlite3_open("/data/adb/magisk.db", &db)) {
                            sql_failed();
                        } else {
                            char *sql_error = 0;
                            const char* sqcmd = ("INSERT INTO hidelist (package_name, process) VALUES (" + appname + ", " + appname + ");").c_str();
                            const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                        }
                        sqlite3_close(db);
                    } else {
                        fprintf(stderr, "Error: app %s not found!\n", appname.c_str());
                        exit(1);
                    }
                }
            } else if (arg_second=="-r") {
                if (argc < 4) {
                    magiskhide_help();
                } else {
                    const string appname = argv[3];
                    struct stat appbuf;
                    const int ifext = stat(("/data/data/" + appname).c_str(), &appbuf);
                    if (ifext==0) {
                        sqlite3 *db = 0;
                        if (sqlite3_open("/data/adb/magisk.db", &db)) {
                            sql_failed();
                        } else {
                            char *sql_error = 0;
                            const char* sqcmd = ("DELETE FROM hidelist where package_name = " + appname + ";").c_str();
                            const auto sql_ret = sqlite3_exec(db, sqcmd, sql_callback, 0, &sql_error);
                        }
                        sqlite3_close(db);
                    } else {
                        fprintf(stderr, "Error: app %s not found!\n", appname.c_str());
                        exit(1);
                    }
                }
            } else {
                magiskhide_help();
            }
        } else if (arg_first=="-b") {
            if (arg_second=="-p") {
                if (argc < 4) {
                    boot_help();
                } else {
                    const string bootimg = argv[3];
                    struct stat bootbuf;
                    const int ifext = stat(bootimg.c_str(), &bootbuf);
                    if (ifext==0) {
                        const bool isfile = S_ISREG(bootbuf.st_mode);
                        if (isfile) {
                            vector<string> magiskfiles;
                            for (auto a : file::directory_iterator("/data/adb/magisk")) {
                                string path = (a.path()).c_str();
                                string fpath = realpath(path.c_str(), NULL);
                                magiskfiles.push_back(fpath);
                            }
                            const unsigned int renamed = rename(bootimg.c_str(), "/data/adb/magisk/boot.img");
                            if (renamed != 0) {
                                permission_denied();
                            }
                            try {
                                chdir("/data/adb/magisk");
                            } catch(...) {
                                permission_denied();
                            }
                            pid_t parent = fork();
                            if (parent) {
                                wait(NULL);
                                for (auto b : file::directory_iterator(".")) {
                                    string path = (b.path()).c_str();
                                    string fpath = realpath(path.c_str(), NULL);
                                    vector<string>::iterator magiskfs;
                                    magiskfs = find(magiskfiles.begin(), magiskfiles.end(), fpath);
                                    if (magiskfs == magiskfiles.end()) {
                                        try {
                                            const bool exist = remove(fpath.c_str());
                                            if (exist) {
                                                rmdir(fpath.c_str());
                                            }
                                        } catch(...) {
                                            permission_denied();
                                        }
                                    }
                                }
                            } else {
                                execlp("sh", "sh", "boot_patch.sh", "./boot.img", NULL);
                            }
                        } else {
                            fprintf(stderr, "%s: not a file\n", bootimg.c_str());
                            exit(1);
                        }
                    } else {
                        nofile();
                    }
                }
            } else if (arg_second=="-u") {
                if (argc < 4) {
                    boot_help();
                } else {
                    const string bootimg = argv[3];
                    struct stat bootbuf;
                    const int ifext = stat(bootimg.c_str(), &bootbuf);
                    if (ifext==0) {
                        const bool isfile = S_ISREG(bootbuf.st_mode);
                        if (isfile) {
                            try {
                                execlp("/data/adb/magisk/magiskboot", "magiskboot", "unpack", bootimg.c_str(), NULL);
                            } catch(...) {
                                permission_denied();
                            }
                        } else {
                            fprintf(stderr, "%s: not a file\n", bootimg.c_str());
                            exit(1);
                        }
                    } else {
                        nofile();
                    }
                }
            } else if (arg_second=="-uh") {
                if (argc < 4) {
                    boot_help();
                } else {
                    const string bootimg = argv[3];
                    struct stat bootbuf;
                    const int ifext = stat(bootimg.c_str(), &bootbuf);
                    if (ifext==0) {
                        const bool isfile = S_ISREG(bootbuf.st_mode);
                        if (isfile) {
                            try {
                                execlp("/data/adb/magisk/magiskboot", "magiskboot", "unpack", "-h", bootimg.c_str(), NULL);
                            } catch(...) {
                                permission_denied();
                            }
                        } else {
                            fprintf(stderr, "%s: not a file\n", bootimg.c_str());
                            exit(1);
                        }
                    } else {
                        nofile();
                    }
                }
            } else if (arg_second=="-r") {
                if (argc < 5) {
                    boot_help();
                } else {
                    const string ramdisk = argv[3];
                    const string arg_four = argv[4];
                    struct stat rambuf;
                    const unsigned int ramext = stat(ramdisk.c_str(), &rambuf);
                    if (ramext==0) {
                        const bool isfile = S_ISREG(rambuf.st_mode);
                        if (isfile) {
                            if (arg_four=="-x") {
                                try {
                                    execlp("/data/adb/magisk/magiskboot", "magiskboot", "cpio", ramdisk.c_str(), "extract", NULL);
                                } catch(...) {
                                    permission_denied();
                                }
                            } else if (arg_four=="-xg") {
                                const string shexec = ("/data/adb/magisk/busybox gzip -kdc " + ramdisk + " | /data/adb/magisk/busybox cpio -i").c_str();
                                sh_exec(shexec.c_str());
                            } else if (arg_four=="-xx") {
                                const string shexec = ("/data/adb/magisk/busybox xz -kdc " + ramdisk + " | /data/adb/magisk/busybox cpio -i").c_str();
                                sh_exec(shexec.c_str());
                            } else if (arg_four=="-d") {
                                if (argc < 6) {
                                    boot_help();
                                } else {
                                    const string ramfile = argv[5];
                                    try {
                                        execlp("/data/adb/magisk/magiskboot", "magiskboot", "cpio", ramdisk, ("rm -r " + ramfile).c_str(), NULL);
                                    } catch(...) {
                                        permission_denied();
                                    }
                                }
                            } else if (arg_four=="-m") {
                                if (argc < 7) {
                                    boot_help();
                                } else {
                                    const string mode = argv[5];
                                    const string file = argv[6];
                                    try {
                                        execlp("/data/adb/magisk/magiskboot", "magiskboot", "cpio", ramdisk, ("mkdir " + mode + " " + file).c_str(), NULL);
                                    } catch(...) {
                                        permission_denied();
                                    }
                                }
                            } else if (arg_four=="-a") {
                                if (argc < 8) {
                                    boot_help();
                                } else {
                                    const string mode = argv[5];
                                    const string file = argv[6];
                                    const string outfile = argv[7];
                                    try {
                                        execlp("/data/adb/magisk/magiskboot", "magiskboot", "cpio", ramdisk, ("add " + mode + " " + file + " " + outfile).c_str(), NULL);
                                    } catch(...) {
                                        permission_denied();
                                    }
                                }
                            } else {
                                boot_help();
                            }
                        } else {
                            fprintf(stderr, "%s: not a file\n", ramdisk.c_str());
                            exit(1);
                        }
                    } else {
                        nofile();
                    }
                }
            } else if (arg_second=="-hx") {
                const string pattern_one = argv[3];
                const string pattern_two = argv[4];
                const string kernel = argv[5];
                struct stat kernelbuf;
                const int ifext = stat(kernel.c_str(), &kernelbuf);
                if (ifext==0) {
                    const bool isfile = S_ISREG(kernelbuf.st_mode);
                    if (isfile) {
                        try {
                            execlp("/data/adb/magisk/magiskboot", "magiskboot" "hexpatch", kernel.c_str(), pattern_one.c_str(), pattern_two.c_str(), NULL);
                        } catch(...) {
                            permission_denied();
                        }
                    } else {
                        fprintf(stderr, "%s: not a file\n", kernel.c_str());
                        exit(1);
                    }
                } else {
                    nofile();
                }
            } else if (arg_second=="-b") {
                const string oldboot = argv[3];
                const string newboot = argv[4];
                struct stat oldbuf;
                const int ifext = stat(oldboot.c_str(), &oldbuf);
                if (ifext==0) {
                    const bool isfile = S_ISREG(oldbuf.st_mode);
                    if (isfile) {
                        try {
                            execlp("/data/adb/magisk/magiskboot", "magiskboot", "repack", oldboot.c_str(), newboot.c_str(), NULL);
                        } catch(...) {
                            permission_denied();
                        }
                    } else {
                        fprintf(stderr, "%s: not a file\n", oldboot.c_str());
                        exit(1);
                    }
                } else {
                    nofile();
                }
            } else {
                boot_help();
            }
        } else if (arg_first=="-c") {
            if (argc < 4) {
                print_help();
            } else {
                const string ramdisk = argv[2];
                const string directory = argv[3];
                struct stat dirbuf;
                const int ifext = stat(directory.c_str(), &dirbuf);
                if (ifext==0) {
                    const bool isdir = S_ISDIR(dirbuf.st_mode);
                    if (isdir) {
                        const string curwd = (file::current_path()).c_str();
                        try {
                            chdir(directory.c_str());
                        } catch(...) {
                            permission_denied();
                        }
                        const string shexec = ("find ./* | cpio -H newc -o > " + (curwd + "/" + ramdisk.c_str())).c_str();
                        sh_exec(shexec.c_str());
                    } else {
                        fprintf(stderr, "%s: Not a directory!\n", directory.c_str());
                        exit(1);
                    }
                } else {
                    fprintf(stderr, "%s: No such directory!\n", directory.c_str());
                    exit(1);
                }
            }
        } else {
            print_help();
        }
    }
    return 0;
}

/* ============================================================================
 * SCos 1.3.5 - Filesystem
 * ============================================================================ */

#include "../include/scos.h"

/* Filesystem storage */
static fs_node_t fs_nodes[FS_MAX_FILES];
static char current_dir[FS_MAX_PATH] = "/";
static int fs_initialized = 0;

/* Find a free node */
static int find_free_node(void) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (!fs_nodes[i].in_use) {
            return i;
        }
    }
    return -1;
}

/* Find node by path */
static int find_node(const char *path) {
    char normalized[FS_MAX_PATH];
    fs_normalize_path(path, normalized);
    
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (fs_nodes[i].in_use) {
            /* Build full path for this node */
            char full_path[FS_MAX_PATH];
            
            if (i == 0) {
                strcpy(full_path, "/");
            } else {
                /* Trace back to root */
                int path_parts[32];
                int num_parts = 0;
                int current = i;
                
                while (current != 0 && num_parts < 32) {
                    path_parts[num_parts++] = current;
                    current = fs_nodes[current].parent_idx;
                }
                
                full_path[0] = '\0';
                for (int j = num_parts - 1; j >= 0; j--) {
                    strcat(full_path, "/");
                    strcat(full_path, fs_nodes[path_parts[j]].name);
                }
            }
            
            if (strcmp(full_path, normalized) == 0) {
                return i;
            }
        }
    }
    
    return -1;
}

/* Normalize path (handle . and ..) */
void fs_normalize_path(const char *path, char *normalized) {
    char temp[FS_MAX_PATH];
    
    /* Handle relative paths */
    if (path[0] != '/') {
        strcpy(temp, current_dir);
        if (temp[strlen(temp) - 1] != '/') {
            strcat(temp, "/");
        }
        strcat(temp, path);
    } else {
        strcpy(temp, path);
    }
    
    /* Parse and normalize */
    char *parts[64];
    int num_parts = 0;
    
    char *token = strtok(temp, "/");
    while (token != NULL && num_parts < 64) {
        if (strcmp(token, ".") == 0) {
            /* Current directory - skip */
        } else if (strcmp(token, "..") == 0) {
            /* Parent directory */
            if (num_parts > 0) {
                num_parts--;
            }
        } else if (strlen(token) > 0) {
            parts[num_parts++] = token;
        }
        token = strtok(NULL, "/");
    }
    
    /* Build normalized path */
    if (num_parts == 0) {
        strcpy(normalized, "/");
    } else {
        normalized[0] = '\0';
        for (int i = 0; i < num_parts; i++) {
            strcat(normalized, "/");
            strcat(normalized, parts[i]);
        }
    }
}

/* Get parent path */
int fs_get_parent_path(const char *path, char *parent) {
    char normalized[FS_MAX_PATH];
    fs_normalize_path(path, normalized);
    
    if (strcmp(normalized, "/") == 0) {
        strcpy(parent, "/");
        return 0;
    }
    
    char *last_slash = strrchr(normalized, '/');
    if (last_slash == normalized) {
        strcpy(parent, "/");
    } else if (last_slash != NULL) {
        int len = last_slash - normalized;
        strncpy(parent, normalized, len);
        parent[len] = '\0';
    } else {
        strcpy(parent, "/");
    }
    
    return 0;
}

/* Get basename */
int fs_get_basename(const char *path, char *name) {
    char normalized[FS_MAX_PATH];
    fs_normalize_path(path, normalized);
    
    if (strcmp(normalized, "/") == 0) {
        strcpy(name, "/");
        return 0;
    }
    
    char *last_slash = strrchr(normalized, '/');
    if (last_slash != NULL) {
        strcpy(name, last_slash + 1);
    } else {
        strcpy(name, normalized);
    }
    
    return 0;
}

/* Initialize filesystem with default structure */
void fs_init(void) {
    /* Clear all nodes */
    memset(fs_nodes, 0, sizeof(fs_nodes));
    
    /* Create root directory */
    fs_nodes[0].in_use = 1;
    strcpy(fs_nodes[0].name, "/");
    fs_nodes[0].type = FS_TYPE_DIR;
    fs_nodes[0].parent_idx = 0;
    fs_nodes[0].created = rtc_get_timestamp();
    fs_nodes[0].modified = fs_nodes[0].created;
    
    /* Create directory structure */
    fs_mkdir("/bin");
    fs_mkdir("/etc");
    fs_mkdir("/home");
    fs_mkdir("/home/user");
    fs_mkdir("/home/user/documents");
    fs_mkdir("/system");
    fs_mkdir("/var");
    fs_mkdir("/var/log");
    
    /* Create default files */
    const char *welcome_txt = 
        "Welcome to SCos 1.3.5!\n"
        "======================\n\n"
        "SCos is a terminal-based operating system designed for simplicity\n"
        "and efficiency. This is a real 32-bit x86 operating system that\n"
        "boots on actual hardware or in QEMU.\n\n"
        "Getting Started:\n"
        "- Type 'help' to see available commands\n"
        "- Use 'ls' to list files and directories\n"
        "- Use 'cd' to change directories\n"
        "- Run 'notepad' to edit text files\n"
        "- Run 'calc' for calculations\n"
        "- Run 'calendar' to view the calendar\n\n"
        "Enjoy using SCos!\n";
    
    fs_touch("/home/user/documents/welcome.txt");
    fs_write("/home/user/documents/welcome.txt", welcome_txt, strlen(welcome_txt));
    
    const char *changelog_txt =
        "SCos 1.3.5 Changelog\n"
        "====================\n\n"
        "Version 1.3.5 (Current)\n"
        "- Complete rewrite as terminal-first OS\n"
        "- Full 32-bit protected mode operation\n"
        "- Custom filesystem implementation\n"
        "- Process scheduler (round-robin)\n"
        "- Memory management with heap allocator\n"
        "- RTC support for date/time\n"
        "- Terminal text editor (notepad)\n"
        "- Calculator application\n"
        "- Calendar application\n"
        "- Network ping simulation\n\n"
        "Version 1.2.0\n"
        "- Initial GUI implementation\n"
        "- Basic window management\n\n"
        "Version 1.0.0\n"
        "- Initial release\n"
        "- Basic boot capability\n";
    
    fs_touch("/home/user/documents/changelog.txt");
    fs_write("/home/user/documents/changelog.txt", changelog_txt, strlen(changelog_txt));
    
    const char *settings_conf =
        "# SCos System Settings\n"
        "# ====================\n\n"
        "[display]\n"
        "text_color=green\n"
        "background_color=black\n"
        "cursor_blink=true\n\n"
        "[system]\n"
        "hostname=scos\n"
        "timezone=UTC\n"
        "autosave=true\n\n"
        "[user]\n"
        "default_user=user\n"
        "home_dir=/home/user\n";
    
    fs_touch("/system/settings.conf");
    fs_write("/system/settings.conf", settings_conf, strlen(settings_conf));
    
    const char *about_txt =
        "SCos - Terminal Operating System\n"
        "=================================\n\n"
        "Version: 1.3.5 \"Terminal\"\n"
        "Architecture: x86 (IA-32)\n"
        "Mode: 32-bit Protected Mode\n"
        "Kernel: Monolithic\n\n"
        "SCos is a minimalist, Linux-inspired operating system\n"
        "designed for educational purposes and hobbyist OS development.\n\n"
        "Features:\n"
        "- Custom bootloader (2-stage)\n"
        "- Protected mode kernel\n"
        "- Memory management\n"
        "- Process scheduling\n"
        "- Custom filesystem\n"
        "- VGA text mode display\n"
        "- PS/2 keyboard input\n"
        "- Real-time clock support\n";
    
    fs_touch("/system/about.txt");
    fs_write("/system/about.txt", about_txt, strlen(about_txt));
    
    const char *network_conf =
        "# SCos Network Configuration\n"
        "# ==========================\n\n"
        "[interface]\n"
        "name=eth0\n"
        "type=ethernet\n"
        "enabled=true\n\n"
        "[ipv4]\n"
        "method=static\n"
        "address=192.168.1.100\n"
        "netmask=255.255.255.0\n"
        "gateway=192.168.1.1\n"
        "dns=8.8.8.8\n\n"
        "[hosts]\n"
        "localhost=127.0.0.1\n"
        "scos=192.168.1.100\n"
        "gateway=192.168.1.1\n"
        "google.com=142.250.80.46\n";
    
    fs_touch("/system/network.conf");
    fs_write("/system/network.conf", network_conf, strlen(network_conf));
    
    /* Set current directory to home */
    strcpy(current_dir, "/home/user");
    
    fs_initialized = 1;
}

/* Create a file or directory */
int fs_create(const char *path, uint8_t type) {
    char normalized[FS_MAX_PATH];
    fs_normalize_path(path, normalized);
    
    /* Check if already exists */
    if (find_node(normalized) >= 0) {
        return -1;  /* Already exists */
    }
    
    /* Get parent path and basename */
    char parent_path[FS_MAX_PATH];
    char name[FS_MAX_NAME];
    fs_get_parent_path(normalized, parent_path);
    fs_get_basename(normalized, name);
    
    /* Find parent node */
    int parent_idx = find_node(parent_path);
    if (parent_idx < 0) {
        return -1;  /* Parent doesn't exist */
    }
    
    if (fs_nodes[parent_idx].type != FS_TYPE_DIR) {
        return -1;  /* Parent is not a directory */
    }
    
    /* Find free node */
    int idx = find_free_node();
    if (idx < 0) {
        return -1;  /* No free nodes */
    }
    
    /* Create node */
    fs_nodes[idx].in_use = 1;
    strncpy(fs_nodes[idx].name, name, FS_MAX_NAME - 1);
    fs_nodes[idx].name[FS_MAX_NAME - 1] = '\0';
    fs_nodes[idx].type = type;
    fs_nodes[idx].size = 0;
    fs_nodes[idx].parent_idx = parent_idx;
    fs_nodes[idx].created = rtc_get_timestamp();
    fs_nodes[idx].modified = fs_nodes[idx].created;
    fs_nodes[idx].content[0] = '\0';
    
    return 0;
}

/* Create directory */
int fs_mkdir(const char *path) {
    return fs_create(path, FS_TYPE_DIR);
}

/* Create empty file */
int fs_touch(const char *path) {
    int idx = find_node(path);
    if (idx >= 0) {
        /* File exists, update modified time */
        fs_nodes[idx].modified = rtc_get_timestamp();
        return 0;
    }
    return fs_create(path, FS_TYPE_FILE);
}

/* Delete file or directory */
int fs_delete(const char *path) {
    char normalized[FS_MAX_PATH];
    fs_normalize_path(path, normalized);
    
    if (strcmp(normalized, "/") == 0) {
        return -1;  /* Cannot delete root */
    }
    
    int idx = find_node(normalized);
    if (idx < 0) {
        return -1;  /* Not found */
    }
    
    /* Check if directory is empty */
    if (fs_nodes[idx].type == FS_TYPE_DIR) {
        for (int i = 0; i < FS_MAX_FILES; i++) {
            if (fs_nodes[i].in_use && fs_nodes[i].parent_idx == idx && i != idx) {
                return -1;  /* Directory not empty */
            }
        }
    }
    
    fs_nodes[idx].in_use = 0;
    return 0;
}

/* Read file content */
int fs_read(const char *path, char *buffer, size_t size) {
    int idx = find_node(path);
    if (idx < 0) {
        return -1;  /* Not found */
    }
    
    if (fs_nodes[idx].type != FS_TYPE_FILE) {
        return -1;  /* Not a file */
    }
    
    size_t to_read = fs_nodes[idx].size;
    if (to_read > size - 1) {
        to_read = size - 1;
    }
    
    memcpy(buffer, fs_nodes[idx].content, to_read);
    buffer[to_read] = '\0';
    
    return (int)to_read;
}

/* Write file content */
int fs_write(const char *path, const char *data, size_t size) {
    int idx = find_node(path);
    if (idx < 0) {
        /* Try to create the file */
        if (fs_touch(path) < 0) {
            return -1;
        }
        idx = find_node(path);
        if (idx < 0) {
            return -1;
        }
    }
    
    if (fs_nodes[idx].type != FS_TYPE_FILE) {
        return -1;  /* Not a file */
    }
    
    size_t to_write = size;
    if (to_write > FS_MAX_CONTENT - 1) {
        to_write = FS_MAX_CONTENT - 1;
    }
    
    memcpy(fs_nodes[idx].content, data, to_write);
    fs_nodes[idx].content[to_write] = '\0';
    fs_nodes[idx].size = to_write;
    fs_nodes[idx].modified = rtc_get_timestamp();
    
    return (int)to_write;
}

/* Append to file */
int fs_append(const char *path, const char *data, size_t size) {
    int idx = find_node(path);
    if (idx < 0) {
        return fs_write(path, data, size);
    }
    
    if (fs_nodes[idx].type != FS_TYPE_FILE) {
        return -1;
    }
    
    size_t current_size = fs_nodes[idx].size;
    size_t to_write = size;
    
    if (current_size + to_write > FS_MAX_CONTENT - 1) {
        to_write = FS_MAX_CONTENT - 1 - current_size;
    }
    
    memcpy(fs_nodes[idx].content + current_size, data, to_write);
    fs_nodes[idx].size = current_size + to_write;
    fs_nodes[idx].content[fs_nodes[idx].size] = '\0';
    fs_nodes[idx].modified = rtc_get_timestamp();
    
    return (int)to_write;
}

/* Check if path exists */
int fs_exists(const char *path) {
    return find_node(path) >= 0;
}

/* Check if path is a directory */
int fs_is_dir(const char *path) {
    int idx = find_node(path);
    if (idx < 0) {
        return 0;
    }
    return fs_nodes[idx].type == FS_TYPE_DIR;
}

/* List directory contents */
int fs_list(const char *path, char *buffer, size_t size) {
    int idx = find_node(path);
    if (idx < 0) {
        return -1;
    }
    
    if (fs_nodes[idx].type != FS_TYPE_DIR) {
        return -1;
    }
    
    buffer[0] = '\0';
    size_t offset = 0;
    
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (fs_nodes[i].in_use && fs_nodes[i].parent_idx == idx && i != idx) {
            size_t name_len = strlen(fs_nodes[i].name);
            if (offset + name_len + 2 < size) {
                strcpy(buffer + offset, fs_nodes[i].name);
                offset += name_len;
                buffer[offset++] = '\n';
                buffer[offset] = '\0';
            }
        }
    }
    
    return (int)offset;
}

/* Change directory */
int fs_chdir(const char *path) {
    char normalized[FS_MAX_PATH];
    fs_normalize_path(path, normalized);
    
    int idx = find_node(normalized);
    if (idx < 0) {
        return -1;  /* Not found */
    }
    
    if (fs_nodes[idx].type != FS_TYPE_DIR) {
        return -1;  /* Not a directory */
    }
    
    strcpy(current_dir, normalized);
    return 0;
}

/* Get current working directory */
char *fs_getcwd(void) {
    return current_dir;
}

/* Get file statistics */
int fs_stat(const char *path, uint32_t *size, uint8_t *type) {
    int idx = find_node(path);
    if (idx < 0) {
        return -1;
    }
    
    if (size) {
        *size = fs_nodes[idx].size;
    }
    if (type) {
        *type = fs_nodes[idx].type;
    }
    
    return 0;
}

/* Save filesystem (placeholder - would need disk driver) */
void fs_save(void) {
    /* In a real implementation, this would write to disk */
}

/* Load filesystem (placeholder - would need disk driver) */
void fs_load(void) {
    /* In a real implementation, this would read from disk */
}

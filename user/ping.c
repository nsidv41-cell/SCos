/* ============================================================================
 * SCos 1.3.5 - Ping Application (Simulated)
 * ============================================================================ */

#include "../include/scos.h"

/* Simulated host database */
typedef struct {
    const char *hostname;
    const char *ip;
    int min_latency;
    int max_latency;
    int packet_loss;  /* Percentage */
} host_info_t;

static host_info_t known_hosts[] = {
    {"localhost",     "127.0.0.1",      0,   1,    0},
    {"scos",          "192.168.1.100",  0,   1,    0},
    {"gateway",       "192.168.1.1",    1,   5,    0},
    {"google.com",    "142.250.80.46",  10,  50,   0},
    {"github.com",    "140.82.112.4",   15,  60,   0},
    {"microsoft.com", "20.70.246.20",   20,  80,   5},
    {"amazon.com",    "54.239.28.85",   25,  100,  2},
    {NULL, NULL, 0, 0, 0}
};

/* Simple pseudo-random number generator */
static uint32_t rand_seed = 12345;

static uint32_t simple_rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (rand_seed / 65536) % 32768;
}

/* Get random number in range */
static int rand_range(int min, int max) {
    if (min >= max) return min;
    return min + (simple_rand() % (max - min + 1));
}

/* Lookup host */
static host_info_t *lookup_host(const char *hostname) {
    for (int i = 0; known_hosts[i].hostname != NULL; i++) {
        if (strcmp(known_hosts[i].hostname, hostname) == 0 ||
            strcmp(known_hosts[i].ip, hostname) == 0) {
            return &known_hosts[i];
        }
    }
    return NULL;
}

/* Resolve hostname (simulated) */
static const char *resolve_host(const char *hostname) {
    host_info_t *host = lookup_host(hostname);
    if (host) {
        return host->ip;
    }
    
    /* Check if it's an IP address */
    int dots = 0;
    for (int i = 0; hostname[i]; i++) {
        if (hostname[i] == '.') dots++;
        else if (!isdigit(hostname[i])) return NULL;
    }
    
    if (dots == 3) {
        return hostname;  /* Assume valid IP */
    }
    
    return NULL;
}

/* Main ping function */
void ping_run(int argc, char **argv) {
    if (argc < 2) {
        vga_puts("Usage: ping <hostname|ip> [-c count]\n");
        vga_puts("\nExamples:\n");
        vga_puts("  ping localhost\n");
        vga_puts("  ping google.com\n");
        vga_puts("  ping 192.168.1.1 -c 5\n");
        return;
    }
    
    const char *hostname = argv[1];
    int count = 4;  /* Default ping count */
    int infinite = 0;
    
    /* Parse arguments */
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            count = atoi(argv[i + 1]);
            if (count <= 0) count = 4;
            i++;
        } else if (strcmp(argv[i], "-t") == 0) {
            infinite = 1;
        }
    }
    
    /* Resolve hostname */
    const char *ip = resolve_host(hostname);
    if (ip == NULL) {
        vga_puts("ping: unknown host ");
        vga_puts(hostname);
        vga_puts("\n");
        return;
    }
    
    /* Get host info for latency simulation */
    host_info_t *host = lookup_host(hostname);
    int min_lat = host ? host->min_latency : 50;
    int max_lat = host ? host->max_latency : 200;
    int loss = host ? host->packet_loss : 10;
    
    /* Seed random with timer */
    rand_seed = timer_get_ticks();
    
    /* Print header */
    char buf[128];
    sprintf(buf, "PING %s (%s) 56(84) bytes of data.\n", hostname, ip);
    vga_puts(buf);
    
    /* Statistics */
    int transmitted = 0;
    int received = 0;
    int min_time = 999999;
    int max_time = 0;
    int total_time = 0;
    
    /* Ping loop */
    int seq = 1;
    while (infinite || seq <= count) {
        transmitted++;
        
        /* Check for Ctrl+C */
        char c = keyboard_getchar_nonblock();
        if (c == 3) {  /* Ctrl+C */
            vga_puts("^C\n");
            break;
        }
        
        /* Simulate packet loss */
        if (rand_range(1, 100) <= loss) {
            /* Packet lost - no output for this one */
            timer_sleep(1000);
            seq++;
            continue;
        }
        
        /* Simulate latency */
        int latency = rand_range(min_lat, max_lat);
        
        /* Simulate network delay */
        timer_sleep(100 + latency);
        
        /* Print result */
        sprintf(buf, "64 bytes from %s: icmp_seq=%d ttl=64 time=%d.%d ms\n",
                ip, seq, latency, rand_range(0, 9));
        vga_puts(buf);
        
        received++;
        if (latency < min_time) min_time = latency;
        if (latency > max_time) max_time = latency;
        total_time += latency;
        
        seq++;
        
        /* Wait between pings (total ~1 second between pings) */
        if (infinite || seq <= count) {
            timer_sleep(900 - latency);
        }
    }
    
    /* Print statistics */
    vga_puts("\n--- ");
    vga_puts(hostname);
    vga_puts(" ping statistics ---\n");
    
    int loss_percent = (transmitted > 0) ? 
                       ((transmitted - received) * 100 / transmitted) : 0;
    
    sprintf(buf, "%d packets transmitted, %d received, %d%% packet loss\n",
            transmitted, received, loss_percent);
    vga_puts(buf);
    
    if (received > 0) {
        int avg = total_time / received;
        sprintf(buf, "rtt min/avg/max = %d.0/%d.%d/%d.0 ms\n",
                min_time, avg, rand_range(0, 9), max_time);
        vga_puts(buf);
    }
}

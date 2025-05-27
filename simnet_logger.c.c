#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// Network interface simulation structure
typedef struct {
    char name[16];
    long long rx_bytes;
    long long tx_bytes;
    long long rx_packets;
    long long tx_packets;
    int rx_errors;
    int tx_errors;
    int status; // 1 = up, 0 = down
    double link_speed; // Mbps
} NetworkInterface;

// Global variables
NetworkInterface interfaces[3];
int interface_count = 3;
int keep_running = 1;
FILE *csv_file = NULL;

// Signal handler for clean shutdown
void signal_handler(int sig) {
    keep_running = 0;
    printf("\nShutting down SimNet Logger...\n");
}

// Initialize network interfaces
void init_interfaces() {
    // Interface 1: Ethernet (eth0)
    strcpy(interfaces[0].name, "eth0");
    interfaces[0].rx_bytes = 1500000;
    interfaces[0].tx_bytes = 800000;
    interfaces[0].rx_packets = 1200;
    interfaces[0].tx_packets = 900;
    interfaces[0].rx_errors = 0;
    interfaces[0].tx_errors = 0;
    interfaces[0].status = 1;
    interfaces[0].link_speed = 1000.0; // 1Gbps

    // Interface 2: WiFi (wlan0)
    strcpy(interfaces[1].name, "wlan0");
    interfaces[1].rx_bytes = 2400000;
    interfaces[1].tx_bytes = 1200000;
    interfaces[1].rx_packets = 1800;
    interfaces[1].tx_packets = 1100;
    interfaces[1].rx_errors = 2;
    interfaces[1].tx_errors = 1;
    interfaces[1].status = 1;
    interfaces[1].link_speed = 150.0; // 150Mbps WiFi

    // Interface 3: Loopback (lo)
    strcpy(interfaces[2].name, "lo");
    interfaces[2].rx_bytes = 45000;
    interfaces[2].tx_bytes = 45000;
    interfaces[2].rx_packets = 120;
    interfaces[2].tx_packets = 120;
    interfaces[2].rx_errors = 0;
    interfaces[2].tx_errors = 0;
    interfaces[2].status = 1;
    interfaces[2].link_speed = 0.0; // Loopback
}

// Generate realistic traffic patterns
int get_traffic_multiplier() {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    // Simulate daily patterns
    int hour = tm_info->tm_hour;

    if (hour >= 9 && hour <= 17) {
        // Business hours - higher traffic
        return 2 + (rand() % 3); // 2-4x multiplier
    } else if (hour >= 18 && hour <= 22) {
        // Evening - medium traffic
        return 1 + (rand() % 2); // 1-2x multiplier
    } else {
        // Night - low traffic
        return rand() % 2; // 0-1x multiplier
    }
}

// Simulate network activity for one interface
void simulate_interface_activity(NetworkInterface *iface, int cycle) {
    if (iface->status == 0) {
        // Interface is down, no activity
        return;
    }

    int traffic_mult = get_traffic_multiplier();

    // Base traffic simulation
    int base_rx_bytes = 1000 + (rand() % 5000);
    int base_tx_bytes = 500 + (rand() % 3000);
    int base_rx_packets = 10 + (rand() % 50);
    int base_tx_packets = 5 + (rand() % 30);

    // Apply traffic multiplier
    base_rx_bytes *= traffic_mult;
    base_tx_bytes *= traffic_mult;
    base_rx_packets *= traffic_mult;
    base_tx_packets *= traffic_mult;

    // Different patterns for different interfaces
    if (strcmp(iface->name, "eth0") == 0) {
        // Ethernet: steady, reliable traffic
        iface->rx_bytes += base_rx_bytes;
        iface->tx_bytes += base_tx_bytes;
        iface->rx_packets += base_rx_packets;
        iface->tx_packets += base_tx_packets;

        // Occasional errors (very rare)
        if (rand() % 100 == 0) {
            iface->rx_errors++;
        }

    } else if (strcmp(iface->name, "wlan0") == 0) {
        // WiFi: more variable, occasional drops
        iface->rx_bytes += base_rx_bytes + (rand() % 2000 - 1000);
        iface->tx_bytes += base_tx_bytes + (rand() % 1500 - 750);
        iface->rx_packets += base_rx_packets + (rand() % 20 - 10);
        iface->tx_packets += base_tx_packets + (rand() % 15 - 7);

        // More frequent errors than ethernet
        if (rand() % 50 == 0) {
            iface->rx_errors++;
        }
        if (rand() % 60 == 0) {
            iface->tx_errors++;
        }

        // Simulate occasional connection issues
        if (rand() % 200 == 0) {
            iface->status = 0; // Go down
            printf("WARNING: %s went down!\n", iface->name);
        }

    } else if (strcmp(iface->name, "lo") == 0) {
        // Loopback: minimal, consistent traffic
        iface->rx_bytes += 100 + (rand() % 200);
        iface->tx_bytes += 100 + (rand() % 200);
        iface->rx_packets += 1 + (rand() % 5);
        iface->tx_packets += 1 + (rand() % 5);
        // Loopback rarely has errors
    }

    // Simulate interface recovery
    if (iface->status == 0 && rand() % 20 == 0) {
        iface->status = 1;
        printf("INFO: %s is back up!\n", iface->name);
    }
}

// Calculate rates (bytes per second, packets per second)
void calculate_rates(NetworkInterface *iface, long long prev_rx_bytes, long long prev_tx_bytes,
                     int prev_rx_packets, int prev_tx_packets, int interval_seconds) {
    // These would be used for rate calculations in a real implementation
    // For simulation, we just generate the traffic directly
}

// Log data to CSV file
void log_to_csv(int cycle) {
    if (csv_file == NULL) return;

    time_t now = time(NULL);

    for (int i = 0; i < interface_count; i++) {
        NetworkInterface *iface = &interfaces[i];

        fprintf(csv_file, "%ld,%d,%s,%lld,%lld,%lld,%lld,%d,%d,%d,%.1f\n",
                now,                    // timestamp
                cycle,                  // cycle number
                iface->name,           // interface name
                iface->rx_bytes,       // received bytes
                iface->tx_bytes,       // transmitted bytes
                iface->rx_packets,     // received packets
                iface->tx_packets,     // transmitted packets
                iface->rx_errors,      // receive errors
                iface->tx_errors,      // transmit errors
                iface->status,         // interface status
                iface->link_speed);    // link speed

        fflush(csv_file);
    }
}

// Display current interface status
void display_status(int cycle) {
    printf("\n=== SimNet Logger - Cycle #%d ===\n", cycle);
    printf("Interface | Status | RX Bytes  | TX Bytes  | RX Pkts | TX Pkts | Errors\n");
    printf("----------|--------|-----------|-----------|---------|---------|--------\n");

    for (int i = 0; i < interface_count; i++) {
        NetworkInterface *iface = &interfaces[i];
        printf("%-9s | %-6s | %9lld | %9lld | %7lld | %7lld | %d/%d\n",
               iface->name,
               iface->status ? "UP" : "DOWN",
               iface->rx_bytes,
               iface->tx_bytes,
               iface->rx_packets,
               iface->tx_packets,
               iface->rx_errors,
               iface->tx_errors);
    }

    // Show some network insights
    long long total_rx = 0, total_tx = 0;
    for (int i = 0; i < interface_count; i++) {
        if (interfaces[i].status) {
            total_rx += interfaces[i].rx_bytes;
            total_tx += interfaces[i].tx_bytes;
        }
    }

    printf("\nTotal Traffic: RX=%.2f MB, TX=%.2f MB\n",
           total_rx / 1024.0 / 1024.0, total_tx / 1024.0 / 1024.0);
}

int main() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    SimNet Logger v1.0                       ║\n");
    printf("║              Network Interface Traffic Simulator             ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    // Set up signal handler
    signal(SIGINT, signal_handler);

    // Initialize random seed
    srand(time(NULL));

    // Initialize CSV file
    csv_file = fopen("simnet_data.csv", "w");
    if (csv_file == NULL) {
        printf("❌ Error: Could not create CSV file\n");
        return 1;
    }

    // Write CSV header
    fprintf(csv_file, "timestamp,cycle,interface,rx_bytes,tx_bytes,rx_packets,tx_packets,rx_errors,tx_errors,status,link_speed\n");
    printf("✓ Created simnet_data.csv for data logging\n");

    // Initialize network interfaces
    init_interfaces();
    printf("✓ Initialized %d network interfaces\n", interface_count);

    printf("🚀 Starting network simulation...\n");
    printf("📊 Logging data every 5 seconds (Press Ctrl+C to stop)\n\n");

    int cycle = 0;

    // Main simulation loop
    while (keep_running) {
        cycle++;

        // Simulate activity for each interface
        for (int i = 0; i < interface_count; i++) {
            simulate_interface_activity(&interfaces[i], cycle);
        }

        // Log data to CSV
        log_to_csv(cycle);

        // Display current status
        display_status(cycle);

        // Wait 5 seconds before next cycle
        sleep(5);
    }

    // Cleanup
    if (csv_file) {
        fclose(csv_file);
        printf("\n✓ Data logged to simnet_data.csv\n");
    }

    printf("📈 Simulation complete! Total cycles: %d\n", cycle);
    printf("🐍 Ready for Python analysis: python3 analyze_simnet.py\n");

    return 0;
}
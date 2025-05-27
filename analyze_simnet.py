#!/usr/bin/env python3
"""
Simple SimNet Data Analyzer
Python script to analyze network simulation data
"""

import pandas as pd
import matplotlib.pyplot as plt

def load_data():
    """Load SimNet CSV data"""
    try:
        df = pd.read_csv('simnet_data.csv')
        print(f"✓ Loaded {len(df)} data points")
        return df
    except FileNotFoundError:
        print("❌ Error: simnet_data.csv not found!")
        return None

def analyze_interfaces(df):
    """Basic analysis of each network interface"""
    print("\n=== INTERFACE ANALYSIS ===")

    for interface in df['interface'].unique():
        data = df[df['interface'] == interface]

        print(f"\n{interface.upper()}:")
        print(f"  Total RX: {data['rx_bytes'].iloc[-1]:,} bytes")
        print(f"  Total TX: {data['tx_bytes'].iloc[-1]:,} bytes")
        print(f"  RX Errors: {data['rx_errors'].iloc[-1]}")
        print(f"  TX Errors: {data['tx_errors'].iloc[-1]}")

        # Calculate total traffic
        total_mb = (data['rx_bytes'].iloc[-1] + data['tx_bytes'].iloc[-1]) / 1024 / 1024
        print(f"  Total Traffic: {total_mb:.2f} MB")

def create_simple_charts(df):
    """Create basic visualizations"""
    print("\n=== CREATING CHARTS ===")

    # Create 2x2 subplot
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 8))
    fig.suptitle('SimNet Network Analysis', fontsize=14)

    # Chart 1: Traffic over time
    for interface in df['interface'].unique():
        data = df[df['interface'] == interface]
        ax1.plot(data['cycle'], data['rx_bytes']/1024/1024, label=f'{interface} RX', marker='o')

    ax1.set_xlabel('Cycle')
    ax1.set_ylabel('Traffic (MB)')
    ax1.set_title('RX Traffic Over Time')
    ax1.legend()
    ax1.grid(True)

    # Chart 2: Total traffic by interface
    interfaces = []
    traffic_totals = []

    for interface in df['interface'].unique():
        data = df[df['interface'] == interface]
        total = (data['rx_bytes'].iloc[-1] + data['tx_bytes'].iloc[-1]) / 1024 / 1024
        interfaces.append(interface)
        traffic_totals.append(total)

    ax2.bar(interfaces, traffic_totals, color=['red', 'blue', 'green'])
    ax2.set_ylabel('Total Traffic (MB)')
    ax2.set_title('Traffic by Interface')

    # Chart 3: Errors over time
    for interface in df['interface'].unique():
        data = df[df['interface'] == interface]
        total_errors = data['rx_errors'] + data['tx_errors']
        ax3.plot(data['cycle'], total_errors, label=interface, marker='s')

    ax3.set_xlabel('Cycle')
    ax3.set_ylabel('Total Errors')
    ax3.set_title('Errors Over Time')
    ax3.legend()
    ax3.grid(True)

    # Chart 4: Packet counts
    for interface in df['interface'].unique():
        data = df[df['interface'] == interface]
        ax4.plot(data['cycle'], data['rx_packets'], label=f'{interface} RX Packets', marker='^')

    ax4.set_xlabel('Cycle')
    ax4.set_ylabel('RX Packets')
    ax4.set_title('Packet Counts Over Time')
    ax4.legend()
    ax4.grid(True)

    plt.tight_layout()
    plt.savefig('network_analysis.png', dpi=150)
    print("✓ Saved charts as 'network_analysis.png'")

    try:
        plt.show()
    except:
        print("  (Display not available)")

def simple_summary(df):
    """Generate basic summary"""
    print("\n=== SUMMARY REPORT ===")

    total_cycles = df['cycle'].max()
    interfaces = len(df['interface'].unique())

    print(f"Simulation ran for {total_cycles} cycles")
    print(f"Monitored {interfaces} network interfaces")

    # Find interface with most traffic
    max_traffic = 0
    busiest_interface = ""

    for interface in df['interface'].unique():
        data = df[df['interface'] == interface]
        total = data['rx_bytes'].iloc[-1] + data['tx_bytes'].iloc[-1]
        if total > max_traffic:
            max_traffic = total
            busiest_interface = interface

    print(f"Busiest interface: {busiest_interface}")
    print(f"Total data transferred: {max_traffic/1024/1024:.2f} MB")

    # Count total errors
    total_errors = 0
    for interface in df['interface'].unique():
        data = df[df['interface'] == interface]
        errors = data['rx_errors'].iloc[-1] + data['tx_errors'].iloc[-1]
        total_errors += errors

    print(f"Total errors detected: {total_errors}")

    if total_errors == 0:
        print("✓ No network errors - system healthy!")
    else:
        print("⚠ Some errors detected - monitoring recommended")

def main():
    """Main function"""
    print("📊 SimNet Data Analyzer")
    print("Simple network data analysis\n")

    # Load data
    df = load_data()
    if df is None:
        return

    # Run analysis
    analyze_interfaces(df)
    create_simple_charts(df)
    simple_summary(df)

    print("\n✅ Analysis complete!")
    print("Generated: network_analysis.png")

if __name__ == "__main__":
    main()
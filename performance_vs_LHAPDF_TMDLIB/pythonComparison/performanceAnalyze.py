import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from scipy import stats
import json
import os
from pathlib import Path

def remove_outliers(df, column, method='iqr', threshold=1.5):
    """
    Remove outliers from a dataframe based on the specified method.
    
    Parameters:
    -----------
    df : pandas DataFrame
        The dataframe containing the data
    column : str
        The column to check for outliers
    method : str, optional (default='iqr')
        Method to use for outlier detection: 'iqr' or 'zscore'
    threshold : float, optional (default=1.5)
        Threshold for outlier detection (1.5 for IQR method, 3 for z-score method)
    
    Returns:
    --------
    pandas DataFrame
        Dataframe with outliers removed
    """
    if method == 'iqr':
        # Group by method to handle different distributions
        result_df = pd.DataFrame()
        for name, group in df.groupby('method'):
            Q1 = group[column].quantile(0.25)
            Q3 = group[column].quantile(0.75)
            IQR = Q3 - Q1
            lower_bound = Q1 - threshold * IQR
            upper_bound = Q3 + threshold * IQR
            
            # Filter out outliers
            filtered_group = group[(group[column] >= lower_bound) & 
                                  (group[column] <= upper_bound)]
            result_df = pd.concat([result_df, filtered_group])
        
        return result_df
    
    elif method == 'zscore':
        # Group by method to handle different distributions
        result_df = pd.DataFrame()
        for name, group in df.groupby('method'):
            z_scores = stats.zscore(group[column])
            filtered_group = group[abs(z_scores) <= threshold]
            result_df = pd.concat([result_df, filtered_group])
        
        return result_df
    
    else:
        raise ValueError("Method must be either 'iqr' or 'zscore'")

def analyze_performance(config_item, output_dir="."):
    """
    Analyze performance data based on configuration.
    
    Parameters:
    -----------
    config_item : dict
        Dictionary containing configuration for a specific analysis
    output_dir : str
        Directory to save output files
    """
    print(f"\nPerforming analysis: {config_item['name']}")
    print("==================================")
    
    # Load data files
    dataframes = []
    
    for i, data_file in enumerate(config_item['data_files']):
        try:
            df = pd.read_csv(data_file)
            # Add method name
            df['method'] = config_item['methods'][i]
            dataframes.append(df)
        except Exception as e:
            print(f"Error loading file {data_file}: {e}")
    
    if not dataframes:
        print("No data loaded. Skipping analysis.")
        return
    
    # Combine all dataframes
    df_combined = pd.concat(dataframes)
    
    # Save original data count
    original_counts = df_combined.groupby('method').size()
    
    # Get outlier settings
    outlier_settings = config_item.get('outlier_settings', {})
    remove_outliers_flag = outlier_settings.get('remove_outliers', True)
    outlier_method = outlier_settings.get('method', 'iqr')
    threshold = outlier_settings.get('threshold', 1.5)
    
    results = {}
    
    # First analyze with outliers removed (default)
    if remove_outliers_flag:
        print(f"\nAnalyzing with outliers removed ({outlier_method.upper()}, threshold={threshold})")
        
        df_filtered = remove_outliers(df_combined, 'time_per_call_ns', 
                                     method=outlier_method, threshold=threshold)
        
        # Calculate how many outliers were removed
        filtered_counts = df_filtered.groupby('method').size()
        removed_counts = original_counts - filtered_counts
        
        print("\nOutlier Removal Summary:")
        print(f"Method used: {outlier_method.upper()}, Threshold: {threshold}")
        for method in original_counts.index:
            removed = removed_counts.get(method, 0)
            print(f"{method}: {removed} outliers removed out of {original_counts[method]} data points "
                  f"({removed/original_counts[method]*100:.2f}%)")
        
        # Analyze filtered data
        create_analysis(df_filtered, config_item, output_dir, "_no_outliers", True)
        results['filtered'] = df_filtered
    
    # Also analyze with outliers included for comparison
    print("\nAnalyzing with all data points (including outliers)")
    create_analysis(df_combined, config_item, output_dir, "", False)
    results['full'] = df_combined
    
    return results

def create_analysis(df, config_item, output_dir, suffix="", outliers_removed=False):
    """
    Create statistical analysis and visualization for the given dataframe.
    
    Parameters:
    -----------
    df : pandas DataFrame
        Dataframe containing the performance data
    config_item : dict
        Dictionary containing configuration
    output_dir : str
        Directory to save output files
    suffix : str
        Suffix to add to output filenames
    outliers_removed : bool
        Whether outliers have been removed from the data
    """
    # Statistical summary
    stats_summary = df.groupby('method')['time_per_call_ns'].describe()
    print("\nPerformance Statistical Summary:")
    print(stats_summary)
    
    # Get each method's data for ANOVA
    methods_data = []
    for method in df['method'].unique():
        methods_data.append(df[df['method'] == method]['time_per_call_ns'])
    
    if all(len(data) > 1 for data in methods_data):
        f_stat, p_value = stats.f_oneway(*methods_data)
        print("\nANOVA Test Results:")
        print(f"F-statistic: {f_stat:.4f}")
        print(f"p-value: {p_value:.4e}")
    else:
        print("\nANOVA Test could not be performed: not enough data points")

    # Create plot
    plt.figure(figsize=config_item.get('figure_size', (20, 15)))
    
    # Set plot style from config
    plot_style = config_item.get('plot_style', {})
    
    sns.boxplot(data=df, 
                x='method', 
                y='time_per_call_ns',
                palette=plot_style.get('palette', 'Set3'),
                width=plot_style.get('width', 0.6),
                fliersize=plot_style.get('fliersize', 5),
                linewidth=plot_style.get('linewidth', 1.5))
    
    # Add title
    title = config_item.get('title', 'Performance Distribution')
    title_suffix = " (Outliers Removed)" if outliers_removed else ""
    
    plt.text(0.98, 0.98, f'{title}{title_suffix}',
             transform=plt.gca().transAxes,
             bbox=dict(facecolor='white', alpha=0.8, edgecolor='gray'),
             verticalalignment='top',
             horizontalalignment='right', 
             fontsize=16)

    # Set y-axis to log scale
    if config_item.get('log_scale', True):
        plt.yscale('log')
    
    # Set labels
    plt.xlabel(config_item.get('xlabel', ''))
    plt.ylabel(config_item.get('ylabel', 'Time per Call (ns)'), fontsize=14)
    
    # Format x-ticks
    plt.xticks(rotation=45, ha='right', fontsize=14)
    
    # Add grid
    plt.grid(True, which="both", ls="-", alpha=0.2)
    
    # Layout and save
    plt.tight_layout()
    
    # Create output filename
    base_filename = config_item.get('output_base', f"perf_analysis_{config_item['name']}")
    output_filename = f"{base_filename}{suffix}.pdf"
    output_path = os.path.join(output_dir, output_filename)
    
    # Create directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close()
    
    # Generate text report
    agg_stats = df.groupby('method')['time_per_call_ns'].agg(['mean', 'std', 'median'])
    fastest_method = agg_stats['mean'].idxmin()
    most_consistent = agg_stats['std'].idxmin()
    
    # Create text report filename
    text_filename = f"{base_filename}{suffix}.txt"
    text_path = os.path.join(output_dir, text_filename)
    
    with open(text_path, 'w') as f:
        f.write(f"Performance Analysis: {title}\n")
        f.write("==================================\n\n")
        
        if outliers_removed:
            f.write("Analysis with outliers removed\n\n")
        else:
            f.write("Analysis with all data points\n\n")
        
        f.write("Performance Statistical Summary:\n")
        f.write(stats_summary.to_string())
        f.write("\n\n")
        
        conclusions = "\nConclusions:\n-------------\n"
        conclusions += f"1. Fastest method (on average): {fastest_method}\n"
        conclusions += f"2. Most consistent method: {most_consistent}\n"
        
        f.write(conclusions)
        
        summary = "\nMethods sorted by average performance (fastest to slowest):\n"
        sorted_by_speed = agg_stats.sort_values('mean')
        
        f.write(summary)
        f.write(sorted_by_speed.to_string())
    
    print(f"\nResults saved to {output_path} and {text_path}")

def main(config_file='performance_config.json'):
    """
    Main function to run the performance analysis based on a JSON config file.
    
    Parameters:
    -----------
    config_file : str
        Path to the JSON configuration file
    """
    try:
        # Load configuration
        with open(config_file, 'r') as f:
            config = json.load(f)
        
        print(f"Loaded configuration from {config_file}")
        
        # Get output directory
        output_dir = config.get('output_directory', 'performance_results')
        
        # Process each analysis configuration
        for analysis_config in config.get('analyses', []):
            analyze_performance(analysis_config, output_dir)
            
    except FileNotFoundError:
        print(f"Configuration file not found: {config_file}")
    except json.JSONDecodeError:
        print(f"Invalid JSON in configuration file: {config_file}")
    except Exception as e:
        print(f"Error processing analysis: {e}")

if __name__ == "__main__":
    import sys
    
    # Use command line argument for config file if provided
    if len(sys.argv) > 1:
        main(sys.argv[1])
    else:
        main()
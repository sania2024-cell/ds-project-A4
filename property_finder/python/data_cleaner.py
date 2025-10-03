"""
Data Cleaning and Preprocessing Script for Property Finder

This script loads raw property data, cleans it, and prepares it for the C++ backend.
"""

import pandas as pd
import numpy as np
from sklearn.preprocessing import LabelEncoder, StandardScaler
import os
import sys

class PropertyDataCleaner:
    def __init__(self):
        self.label_encoders = {}
        self.scaler = StandardScaler()
        
    def load_raw_data(self, filename):
        """Load raw property data from CSV file"""
        try:
            print(f"Loading data from {filename}...")
            df = pd.read_csv(filename)
            print(f"Loaded {len(df)} rows with {len(df.columns)} columns")
            return df
        except Exception as e:
            print(f"Error loading data: {e}")
            return None
    
    def clean_data(self, df):
        """Clean and preprocess the property data"""
        print("Starting data cleaning process...")
        
        # Make a copy to avoid modifying original
        cleaned_df = df.copy()
        
        # Basic info about the dataset
        print(f"Initial shape: {cleaned_df.shape}")
        print(f"Missing values per column:\n{cleaned_df.isnull().sum()}")
        
        # Handle missing values
        cleaned_df = self._handle_missing_values(cleaned_df)
        
        # Clean price column (remove currency symbols, commas)
        if 'Price' in cleaned_df.columns:
            cleaned_df['Price'] = self._clean_price_column(cleaned_df['Price'])
        
        # Clean size column
        if 'Size' in cleaned_df.columns:
            cleaned_df['Size'] = self._clean_size_column(cleaned_df['Size'])
        
        # Standardize city names
        if 'City' in cleaned_df.columns:
            cleaned_df['City'] = self._standardize_city_names(cleaned_df['City'])
        
        # Clean property type
        if 'Type' in cleaned_df.columns:
            cleaned_df['Type'] = self._standardize_property_types(cleaned_df['Type'])
        
        # Validate coordinates
        if 'Latitude' in cleaned_df.columns and 'Longitude' in cleaned_df.columns:
            cleaned_df = self._validate_coordinates(cleaned_df)
        
        # Remove duplicates
        initial_count = len(cleaned_df)
        cleaned_df = cleaned_df.drop_duplicates()
        removed_duplicates = initial_count - len(cleaned_df)
        if removed_duplicates > 0:
            print(f"Removed {removed_duplicates} duplicate rows")
        
        print(f"Final shape after cleaning: {cleaned_df.shape}")
        return cleaned_df
    
    def _handle_missing_values(self, df):
        """Handle missing values in the dataset"""
        print("Handling missing values...")
        
        # Fill missing bedrooms/bathrooms with median
        numeric_columns = ['Bedrooms', 'Bathrooms']
        for col in numeric_columns:
            if col in df.columns:
                median_val = df[col].median()
                df[col].fillna(median_val, inplace=True)
                print(f"Filled {col} missing values with median: {median_val}")
        
        # Fill missing size with median based on bedrooms
        if 'Size' in df.columns and 'Bedrooms' in df.columns:
            for bedrooms in df['Bedrooms'].unique():
                if not pd.isna(bedrooms):
                    mask = (df['Bedrooms'] == bedrooms) & df['Size'].isna()
                    if mask.any():
                        median_size = df[df['Bedrooms'] == bedrooms]['Size'].median()
                        df.loc[mask, 'Size'] = median_size
        
        # Fill remaining missing sizes with overall median
        if 'Size' in df.columns:
            df['Size'].fillna(df['Size'].median(), inplace=True)
        
        # Fill missing categorical values with mode
        categorical_columns = ['City', 'Type']
        for col in categorical_columns:
            if col in df.columns:
                mode_val = df[col].mode().iloc[0] if not df[col].mode().empty else 'Unknown'
                df[col].fillna(mode_val, inplace=True)
                print(f"Filled {col} missing values with mode: {mode_val}")
        
        # Drop rows with missing essential information
        essential_columns = ['Price', 'City']
        for col in essential_columns:
            if col in df.columns:
                before_count = len(df)
                df = df.dropna(subset=[col])
                removed_count = before_count - len(df)
                if removed_count > 0:
                    print(f"Removed {removed_count} rows with missing {col}")
        
        return df
    
    def _clean_price_column(self, price_series):
        """Clean price column by removing currency symbols and converting to numeric"""
        print("Cleaning price column...")
        
        # Convert to string and handle various formats
        prices = price_series.astype(str)
        
        # Remove currency symbols and commas
        prices = prices.str.replace('₹', '', regex=False)
        prices = prices.str.replace('Rs', '', regex=False)
        prices = prices.str.replace(',', '', regex=False)
        prices = prices.str.replace(' ', '', regex=False)
        
        # Handle 'Crore' and 'Lakh' conversions
        crore_mask = prices.str.contains('Crore|crore', na=False)
        lakh_mask = prices.str.contains('Lakh|lakh', na=False)
        
        prices.loc[crore_mask] = prices.loc[crore_mask].str.replace('Crore|crore', '', regex=True).astype(float) * 10000000
        prices.loc[lakh_mask] = prices.loc[lakh_mask].str.replace('Lakh|lakh', '', regex=True).astype(float) * 100000
        
        # Convert remaining to numeric
        prices = pd.to_numeric(prices, errors='coerce')
        
        # Remove unrealistic prices (too low or too high)
        valid_prices = prices[(prices >= 100000) & (prices <= 1000000000)]
        print(f"Price range: {valid_prices.min():,.0f} to {valid_prices.max():,.0f}")
        
        return prices
    
    def _clean_size_column(self, size_series):
        """Clean size column and convert to numeric (square feet)"""
        print("Cleaning size column...")
        
        sizes = size_series.astype(str)
        
        # Remove common suffixes
        sizes = sizes.str.replace('sq ft|sqft|sq.ft|sq feet|square feet', '', regex=True)
        sizes = sizes.str.replace(' ', '', regex=False)
        
        # Convert to numeric
        sizes = pd.to_numeric(sizes, errors='coerce')
        
        # Remove unrealistic sizes
        valid_sizes = sizes[(sizes >= 200) & (sizes <= 10000)]
        print(f"Size range: {valid_sizes.min():.0f} to {valid_sizes.max():.0f} sq ft")
        
        return sizes
    
    def _standardize_city_names(self, city_series):
        """Standardize city names"""
        print("Standardizing city names...")
        
        cities = city_series.str.strip().str.title()
        
        # Common city name mappings
        city_mappings = {
            'Bombay': 'Mumbai',
            'Bangalore': 'Bengaluru',
            'Calcutta': 'Kolkata',
            'Madras': 'Chennai',
            'New Delhi': 'Delhi',
            'Gurgaon': 'Gurugram'
        }
        
        for old_name, new_name in city_mappings.items():
            cities = cities.str.replace(old_name, new_name, regex=False)
        
        print(f"Unique cities: {cities.nunique()}")
        print(f"Top 10 cities: {cities.value_counts().head(10).to_dict()}")
        
        return cities
    
    def _standardize_property_types(self, type_series):
        """Standardize property types"""
        print("Standardizing property types...")
        
        types = type_series.str.strip().str.title()
        
        # Common property type mappings
        type_mappings = {
            'Flat': 'Apartment',
            'Builder Floor': 'Apartment',
            'Independent House': 'House',
            'Villa': 'House',
            'Plot': 'Land'
        }
        
        for old_type, new_type in type_mappings.items():
            types = types.str.replace(old_type, new_type, regex=False)
        
        print(f"Property types: {types.value_counts().to_dict()}")
        
        return types
    
    def _validate_coordinates(self, df):
        """Validate latitude and longitude coordinates"""
        print("Validating coordinates...")
        
        # India coordinate bounds (approximate)
        lat_min, lat_max = 8.0, 37.0
        lon_min, lon_max = 68.0, 97.0
        
        valid_coords = (
            (df['Latitude'] >= lat_min) & (df['Latitude'] <= lat_max) &
            (df['Longitude'] >= lon_min) & (df['Longitude'] <= lon_max)
        )
        
        invalid_count = len(df) - valid_coords.sum()
        if invalid_count > 0:
            print(f"Found {invalid_count} properties with invalid coordinates")
            df = df[valid_coords]
        
        return df
    
    def add_features(self, df):
        """Add derived features for ML"""
        print("Adding derived features...")
        
        # Price per square foot
        if 'Price' in df.columns and 'Size' in df.columns:
            df['Price_per_sqft'] = df['Price'] / df['Size']
        
        # Bathroom to bedroom ratio
        if 'Bathrooms' in df.columns and 'Bedrooms' in df.columns:
            df['Bath_Bed_Ratio'] = df['Bathrooms'] / df['Bedrooms'].replace(0, 1)
        
        # Size category
        if 'Size' in df.columns:
            df['Size_Category'] = pd.cut(df['Size'], 
                                       bins=[0, 500, 1000, 2000, float('inf')],
                                       labels=['Small', 'Medium', 'Large', 'XLarge'])
        
        # Price category
        if 'Price' in df.columns:
            df['Price_Category'] = pd.qcut(df['Price'], 
                                         q=4, 
                                         labels=['Budget', 'Mid-Range', 'Premium', 'Luxury'])
        
        print(f"Added {4} derived features")
        return df
    
    def save_cleaned_data(self, df, output_filename):
        """Save cleaned data to CSV"""
        try:
            # Ensure output directory exists
            os.makedirs(os.path.dirname(output_filename), exist_ok=True)
            
            df.to_csv(output_filename, index=False)
            print(f"Cleaned data saved to {output_filename}")
            print(f"Final dataset shape: {df.shape}")
            return True
        except Exception as e:
            print(f"Error saving data: {e}")
            return False
    
    def generate_summary_report(self, df, output_dir):
        """Generate a summary report of the cleaned data"""
        report_filename = os.path.join(output_dir, 'data_summary_report.txt')
        
        with open(report_filename, 'w') as f:
            f.write("PROPERTY DATA CLEANING SUMMARY REPORT\n")
            f.write("=" * 50 + "\n\n")
            
            f.write(f"Dataset Shape: {df.shape}\n")
            f.write(f"Total Properties: {len(df)}\n\n")
            
            f.write("COLUMN INFORMATION:\n")
            f.write("-" * 20 + "\n")
            f.write(f"{df.info(buf=f)}\n\n")
            
            f.write("MISSING VALUES:\n")
            f.write("-" * 15 + "\n")
            f.write(f"{df.isnull().sum()}\n\n")
            
            f.write("NUMERICAL STATISTICS:\n")
            f.write("-" * 22 + "\n")
            f.write(f"{df.describe()}\n\n")
            
            if 'City' in df.columns:
                f.write("TOP 10 CITIES:\n")
                f.write("-" * 14 + "\n")
                f.write(f"{df['City'].value_counts().head(10)}\n\n")
            
            if 'Type' in df.columns:
                f.write("PROPERTY TYPES:\n")
                f.write("-" * 15 + "\n")
                f.write(f"{df['Type'].value_counts()}\n\n")
        
        print(f"Summary report saved to {report_filename}")

def main():
    """Main function to run data cleaning process"""
    
    # Configuration
    raw_data_file = "../data/raw_properties.csv"
    cleaned_data_file = "../data/cleaned_properties.csv"
    
    # Check if raw data file exists, if not create sample data
    if not os.path.exists(raw_data_file):
        print(f"Raw data file not found: {raw_data_file}")
        print("Creating sample dataset...")
        create_sample_dataset(raw_data_file)
    
    # Initialize cleaner
    cleaner = PropertyDataCleaner()
    
    # Load raw data
    raw_df = cleaner.load_raw_data(raw_data_file)
    if raw_df is None:
        return
    
    # Clean data
    cleaned_df = cleaner.clean_data(raw_df)
    
    # Add features
    cleaned_df = cleaner.add_features(cleaned_df)
    
    # Save cleaned data
    if cleaner.save_cleaned_data(cleaned_df, cleaned_data_file):
        # Generate summary report
        cleaner.generate_summary_report(cleaned_df, "../data")
        
        print("\nData cleaning completed successfully!")
        print(f"Cleaned dataset: {cleaned_data_file}")
        print("Ready for C++ backend processing.")
    
def create_sample_dataset(filename):
    """Create a sample dataset for testing"""
    print("Creating sample property dataset...")
    
    np.random.seed(42)
    n_properties = 1000
    
    cities = ['Mumbai', 'Delhi', 'Bengaluru', 'Chennai', 'Kolkata', 'Hyderabad', 'Pune', 'Ahmedabad']
    property_types = ['Apartment', 'House', 'Villa', 'Studio']
    
    # Generate sample data
    data = {
        'ID': range(1, n_properties + 1),
        'City': np.random.choice(cities, n_properties),
        'Price': np.random.lognormal(15, 0.5, n_properties).astype(int),
        'Bedrooms': np.random.choice([1, 2, 3, 4, 5], n_properties, p=[0.1, 0.3, 0.4, 0.15, 0.05]),
        'Bathrooms': np.random.randint(1, 4, n_properties),
        'Size': np.random.normal(1200, 400, n_properties).astype(int),
        'Type': np.random.choice(property_types, n_properties),
        'Latitude': np.random.uniform(8.0, 35.0, n_properties),
        'Longitude': np.random.uniform(68.0, 97.0, n_properties),
        'Amenities': np.random.choice(['Gym,Pool', 'Parking', 'Garden', 'Security', 'Gym'], n_properties)
    }
    
    # Create DataFrame and save
    df = pd.DataFrame(data)
    
    # Ensure directory exists
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    
    df.to_csv(filename, index=False)
    print(f"Sample dataset created: {filename}")
    print(f"Generated {n_properties} sample properties")

if __name__ == "__main__":
    main()
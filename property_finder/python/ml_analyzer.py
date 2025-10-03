"""
Machine Learning Prototype and Analysis Script

This script develops and tests ML models for property price prediction
and provides insights for C++ implementation.
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.linear_model import LinearRegression, Ridge, Lasso
from sklearn.ensemble import RandomForestRegressor, GradientBoostingRegressor
from sklearn.preprocessing import StandardScaler, LabelEncoder
from sklearn.metrics import mean_squared_error, r2_score, mean_absolute_error
import joblib
import os
import warnings
warnings.filterwarnings('ignore')

class PropertyMLAnalyzer:
    def __init__(self):
        self.models = {}
        self.scalers = {}
        self.encoders = {}
        self.feature_columns = []
        self.target_column = 'Price'
        
    def load_data(self, filename):
        """Load cleaned property data"""
        try:
            print(f"Loading data from {filename}...")
            self.df = pd.read_csv(filename)
            print(f"Loaded {len(self.df)} properties with {len(self.df.columns)} columns")
            print(f"Columns: {list(self.df.columns)}")
            return True
        except Exception as e:
            print(f"Error loading data: {e}")
            return False
    
    def explore_data(self, save_plots=True):
        """Perform exploratory data analysis"""
        print("\n=== EXPLORATORY DATA ANALYSIS ===")
        
        # Basic statistics
        print(f"\nDataset Info:")
        print(f"Shape: {self.df.shape}")
        print(f"Missing values: {self.df.isnull().sum().sum()}")
        
        # Target variable analysis
        if self.target_column in self.df.columns:
            price_stats = self.df[self.target_column].describe()
            print(f"\nPrice Statistics:")
            print(price_stats)
            
        # Create visualizations
        if save_plots:
            self._create_eda_plots()
    
    def _create_eda_plots(self):
        """Create EDA visualizations"""
        os.makedirs('../docs/plots', exist_ok=True)
        
        # Set style
        plt.style.use('seaborn-v0_8')
        sns.set_palette("husl")
        
        # 1. Price distribution
        plt.figure(figsize=(12, 8))
        
        plt.subplot(2, 2, 1)
        plt.hist(self.df[self.target_column], bins=50, alpha=0.7, edgecolor='black')
        plt.title('Price Distribution')
        plt.xlabel('Price (₹)')
        plt.ylabel('Frequency')
        
        plt.subplot(2, 2, 2)
        plt.hist(np.log(self.df[self.target_column]), bins=50, alpha=0.7, edgecolor='black')
        plt.title('Log Price Distribution')
        plt.xlabel('Log(Price)')
        plt.ylabel('Frequency')
        
        # 3. City-wise price analysis
        plt.subplot(2, 2, 3)
        city_prices = self.df.groupby('City')[self.target_column].median().sort_values(ascending=False)
        city_prices.head(10).plot(kind='bar')
        plt.title('Median Price by City (Top 10)')
        plt.xlabel('City')
        plt.ylabel('Median Price (₹)')
        plt.xticks(rotation=45)
        
        # 4. Bedrooms vs Price
        plt.subplot(2, 2, 4)
        bedroom_prices = self.df.groupby('Bedrooms')[self.target_column].median()
        bedroom_prices.plot(kind='bar')
        plt.title('Median Price by Bedrooms')
        plt.xlabel('Bedrooms')
        plt.ylabel('Median Price (₹)')
        
        plt.tight_layout()
        plt.savefig('../docs/plots/price_analysis.png', dpi=300, bbox_inches='tight')
        plt.show()
        
        # Correlation heatmap
        plt.figure(figsize=(10, 8))
        numeric_columns = self.df.select_dtypes(include=[np.number]).columns
        correlation_matrix = self.df[numeric_columns].corr()
        
        sns.heatmap(correlation_matrix, annot=True, cmap='coolwarm', center=0,
                   square=True, fmt='.2f')
        plt.title('Feature Correlation Matrix')
        plt.tight_layout()
        plt.savefig('../docs/plots/correlation_matrix.png', dpi=300, bbox_inches='tight')
        plt.show()
        
        # Property type distribution
        plt.figure(figsize=(12, 6))
        
        plt.subplot(1, 2, 1)
        self.df['Type'].value_counts().plot(kind='pie', autopct='%1.1f%%')
        plt.title('Property Type Distribution')
        
        plt.subplot(1, 2, 2)
        sns.boxplot(data=self.df, x='Type', y=self.target_column)
        plt.title('Price Distribution by Property Type')
        plt.xticks(rotation=45)
        
        plt.tight_layout()
        plt.savefig('../docs/plots/property_type_analysis.png', dpi=300, bbox_inches='tight')
        plt.show()
        
        print("EDA plots saved to ../docs/plots/")
    
    def prepare_features(self):
        """Prepare features for machine learning"""
        print("\n=== FEATURE ENGINEERING ===")
        
        # Identify feature columns
        self.feature_columns = ['City', 'Type', 'Bedrooms', 'Bathrooms', 'Size', 'Latitude', 'Longitude']
        
        # Remove rows with missing target
        self.df = self.df.dropna(subset=[self.target_column])
        
        # Create feature matrix
        X = self.df[self.feature_columns].copy()
        y = self.df[self.target_column].copy()
        
        # Handle categorical variables
        categorical_columns = ['City', 'Type']
        
        for col in categorical_columns:
            if col in X.columns:
                le = LabelEncoder()
                X[col] = le.fit_transform(X[col].astype(str))
                self.encoders[col] = le
                print(f"Encoded {col}: {len(le.classes_)} categories")
        
        # Add derived features
        if 'Size' in X.columns and 'Bedrooms' in X.columns:
            X['Size_per_Bedroom'] = X['Size'] / X['Bedrooms'].replace(0, 1)
        
        if 'Bathrooms' in X.columns and 'Bedrooms' in X.columns:
            X['Bath_Bed_Ratio'] = X['Bathrooms'] / X['Bedrooms'].replace(0, 1)
        
        # Scale numerical features
        numerical_columns = X.select_dtypes(include=[np.number]).columns
        scaler = StandardScaler()
        X[numerical_columns] = scaler.fit_transform(X[numerical_columns])
        self.scalers['features'] = scaler
        
        print(f"Final feature matrix shape: {X.shape}")
        print(f"Features: {list(X.columns)}")
        
        return X, y
    
    def train_models(self, X, y):
        """Train multiple ML models"""
        print("\n=== MODEL TRAINING ===")
        
        # Split data
        X_train, X_test, y_train, y_test = train_test_split(
            X, y, test_size=0.2, random_state=42
        )
        
        # Define models
        models_to_train = {
            'Linear Regression': LinearRegression(),
            'Ridge Regression': Ridge(alpha=1.0),
            'Lasso Regression': Lasso(alpha=1.0),
            'Random Forest': RandomForestRegressor(n_estimators=100, random_state=42),
            'Gradient Boosting': GradientBoostingRegressor(n_estimators=100, random_state=42)
        }
        
        results = []
        
        for name, model in models_to_train.items():
            print(f"\nTraining {name}...")
            
            # Train model
            model.fit(X_train, y_train)
            self.models[name] = model
            
            # Predictions
            y_pred_train = model.predict(X_train)
            y_pred_test = model.predict(X_test)
            
            # Metrics
            train_r2 = r2_score(y_train, y_pred_train)
            test_r2 = r2_score(y_test, y_pred_test)
            train_mae = mean_absolute_error(y_train, y_pred_train)
            test_mae = mean_absolute_error(y_test, y_pred_test)
            train_rmse = np.sqrt(mean_squared_error(y_train, y_pred_train))
            test_rmse = np.sqrt(mean_squared_error(y_test, y_pred_test))
            
            # Cross-validation
            cv_scores = cross_val_score(model, X_train, y_train, cv=5, scoring='r2')
            
            results.append({
                'Model': name,
                'Train R²': train_r2,
                'Test R²': test_r2,
                'Train MAE': train_mae,
                'Test MAE': test_mae,
                'Train RMSE': train_rmse,
                'Test RMSE': test_rmse,
                'CV R² Mean': cv_scores.mean(),
                'CV R² Std': cv_scores.std()
            })
            
            print(f"  Test R²: {test_r2:.4f}")
            print(f"  Test MAE: ₹{test_mae:,.0f}")
            print(f"  Test RMSE: ₹{test_rmse:,.0f}")
        
        # Create results DataFrame
        results_df = pd.DataFrame(results)
        print("\n=== MODEL COMPARISON ===")
        print(results_df.round(4))
        
        # Save results
        results_df.to_csv('../docs/model_comparison.csv', index=False)
        
        return X_train, X_test, y_train, y_test, results_df
    
    def analyze_best_model(self, X_test, y_test, results_df):
        """Analyze the best performing model"""
        print("\n=== BEST MODEL ANALYSIS ===")
        
        # Find best model
        best_model_name = results_df.loc[results_df['Test R²'].idxmax(), 'Model']
        best_model = self.models[best_model_name]
        
        print(f"Best Model: {best_model_name}")
        
        # Feature importance (if available)
        if hasattr(best_model, 'feature_importances_'):
            feature_names = X_test.columns
            importances = best_model.feature_importances_
            
            feature_importance_df = pd.DataFrame({
                'Feature': feature_names,
                'Importance': importances
            }).sort_values('Importance', ascending=False)
            
            print("\nFeature Importances:")
            print(feature_importance_df)
            
            # Plot feature importance
            plt.figure(figsize=(10, 6))
            sns.barplot(data=feature_importance_df.head(10), x='Importance', y='Feature')
            plt.title(f'Top 10 Feature Importances - {best_model_name}')
            plt.tight_layout()
            plt.savefig('../docs/plots/feature_importance.png', dpi=300, bbox_inches='tight')
            plt.show()
        
        elif hasattr(best_model, 'coef_'):
            # Linear model coefficients
            feature_names = X_test.columns
            coefficients = best_model.coef_
            
            coef_df = pd.DataFrame({
                'Feature': feature_names,
                'Coefficient': coefficients
            }).sort_values('Coefficient', key=abs, ascending=False)
            
            print("\nModel Coefficients:")
            print(coef_df)
            
            # Export for C++ implementation
            self._export_linear_model_params(best_model, feature_names)
        
        # Prediction vs Actual plot
        y_pred = best_model.predict(X_test)
        
        plt.figure(figsize=(10, 6))
        plt.scatter(y_test, y_pred, alpha=0.6)
        plt.plot([y_test.min(), y_test.max()], [y_test.min(), y_test.max()], 'r--', lw=2)
        plt.xlabel('Actual Price (₹)')
        plt.ylabel('Predicted Price (₹)')
        plt.title(f'Actual vs Predicted Prices - {best_model_name}')
        plt.tight_layout()
        plt.savefig('../docs/plots/prediction_scatter.png', dpi=300, bbox_inches='tight')
        plt.show()
        
        return best_model_name, best_model
    
    def _export_linear_model_params(self, model, feature_names):
        """Export linear model parameters for C++ implementation"""
        print("\n=== EXPORTING MODEL FOR C++ ===")
        
        model_params = {
            'model_type': 'linear_regression',
            'intercept': float(model.intercept_),
            'coefficients': model.coef_.tolist(),
            'feature_names': list(feature_names),
            'feature_means': self.scalers['features'].mean_.tolist(),
            'feature_scales': self.scalers['features'].scale_.tolist()
        }
        
        # Save as text file for easy C++ loading
        with open('../data/model_params.txt', 'w') as f:
            f.write(f"model_type={model_params['model_type']}\n")
            f.write(f"intercept={model_params['intercept']}\n")
            f.write("coefficients=")
            f.write(','.join(map(str, model_params['coefficients'])))
            f.write('\n')
            f.write("feature_names=")
            f.write(','.join(model_params['feature_names']))
            f.write('\n')
            f.write("feature_means=")
            f.write(','.join(map(str, model_params['feature_means'])))
            f.write('\n')
            f.write("feature_scales=")
            f.write(','.join(map(str, model_params['feature_scales'])))
            f.write('\n')
        
        # Also save categorical encoders
        with open('../data/encoders.txt', 'w') as f:
            for col, encoder in self.encoders.items():
                f.write(f"{col}_classes=")
                f.write(','.join(encoder.classes_))
                f.write('\n')
        
        print("Model parameters exported for C++ implementation:")
        print("  - ../data/model_params.txt")
        print("  - ../data/encoders.txt")
    
    def save_models(self):
        """Save trained models"""
        model_dir = '../data/models'
        os.makedirs(model_dir, exist_ok=True)
        
        for name, model in self.models.items():
            filename = os.path.join(model_dir, f"{name.lower().replace(' ', '_')}.joblib")
            joblib.dump(model, filename)
            print(f"Saved {name} to {filename}")
        
        # Save scalers and encoders
        joblib.dump(self.scalers, os.path.join(model_dir, 'scalers.joblib'))
        joblib.dump(self.encoders, os.path.join(model_dir, 'encoders.joblib'))
    
    def generate_ml_report(self, results_df, best_model_name):
        """Generate comprehensive ML analysis report"""
        report_filename = '../docs/ml_analysis_report.md'
        
        with open(report_filename, 'w') as f:
            f.write("# Property Price Prediction - ML Analysis Report\n\n")
            
            f.write("## Dataset Overview\n")
            f.write(f"- **Total Properties**: {len(self.df):,}\n")
            f.write(f"- **Features Used**: {len(self.feature_columns)}\n")
            f.write(f"- **Target Variable**: {self.target_column}\n\n")
            
            f.write("## Model Performance Comparison\n\n")
            f.write("| Model | Test R² | Test MAE (₹) | Test RMSE (₹) |\n")
            f.write("|-------|---------|--------------|---------------|\n")
            
            for _, row in results_df.iterrows():
                f.write(f"| {row['Model']} | {row['Test R²']:.4f} | "
                       f"₹{row['Test MAE']:,.0f} | ₹{row['Test RMSE']:,.0f} |\n")
            
            f.write(f"\n## Best Model: {best_model_name}\n\n")
            
            best_row = results_df[results_df['Model'] == best_model_name].iloc[0]
            f.write(f"- **R² Score**: {best_row['Test R²']:.4f}\n")
            f.write(f"- **Mean Absolute Error**: ₹{best_row['Test MAE']:,.0f}\n")
            f.write(f"- **Root Mean Square Error**: ₹{best_row['Test RMSE']:,.0f}\n")
            f.write(f"- **Cross-Validation R²**: {best_row['CV R² Mean']:.4f} ± {best_row['CV R² Std']:.4f}\n\n")
            
            f.write("## Implementation Notes for C++\n\n")
            f.write("1. **Model Type**: Linear Regression (easiest to implement)\n")
            f.write("2. **Feature Scaling**: StandardScaler normalization required\n")
            f.write("3. **Categorical Encoding**: Label encoding for City and Type\n")
            f.write("4. **Key Features**: Size, Bedrooms, City, Type show highest importance\n\n")
            
            f.write("## Files Generated\n\n")
            f.write("- `model_params.txt`: Model coefficients and parameters\n")
            f.write("- `encoders.txt`: Categorical variable encodings\n")
            f.write("- `plots/`: Visualization files\n")
            f.write("- `model_comparison.csv`: Detailed model comparison\n")
        
        print(f"ML analysis report saved to {report_filename}")

def main():
    """Main analysis pipeline"""
    
    # Configuration
    data_file = "../data/cleaned_properties.csv"
    
    # Initialize analyzer
    analyzer = PropertyMLAnalyzer()
    
    # Load data
    if not analyzer.load_data(data_file):
        print("Failed to load data. Please run data_cleaner.py first.")
        return
    
    # Exploratory Data Analysis
    analyzer.explore_data(save_plots=True)
    
    # Prepare features
    X, y = analyzer.prepare_features()
    
    # Train models
    X_train, X_test, y_train, y_test, results_df = analyzer.train_models(X, y)
    
    # Analyze best model
    best_model_name, best_model = analyzer.analyze_best_model(X_test, y_test, results_df)
    
    # Save models
    analyzer.save_models()
    
    # Generate report
    analyzer.generate_ml_report(results_df, best_model_name)
    
    print("\n" + "="*50)
    print("ML ANALYSIS COMPLETED SUCCESSFULLY!")
    print("="*50)
    print("Ready for C++ implementation with exported parameters.")

if __name__ == "__main__":
    main()
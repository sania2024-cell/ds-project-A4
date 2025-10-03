#!/usr/bin/env python3
"""
Test script to verify all Python dependencies are properly installed
"""

import sys

def test_imports():
    """Test all required imports"""
    try:
        import pandas as pd
        print("✓ pandas imported successfully")
        
        import numpy as np  
        print("✓ numpy imported successfully")
        
        import matplotlib.pyplot as plt
        print("✓ matplotlib imported successfully")
        
        import seaborn as sns
        print("✓ seaborn imported successfully")
        
        from sklearn.model_selection import train_test_split
        from sklearn.linear_model import LinearRegression
        from sklearn.ensemble import RandomForestRegressor
        from sklearn.preprocessing import StandardScaler, LabelEncoder
        from sklearn.metrics import mean_squared_error, r2_score
        print("✓ scikit-learn imported successfully")
        
        import joblib
        print("✓ joblib imported successfully")
        
        print("\n🎉 All Python dependencies are installed and working!")
        return True
        
    except ImportError as e:
        print(f"❌ Import error: {e}")
        return False
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
        return False

if __name__ == "__main__":
    success = test_imports()
    sys.exit(0 if success else 1)
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.ensemble import RandomForestRegressor
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_squared_error, r2_score

import pandas as pd

import pandas as pd

def preprocess_data(file_path):
    # Initialize lists to hold the sensor names and values
    sensor_names = []
    sensor_values = []

    # Read the file line by line
    with open(file_path, 'r') as file:
        for line in file:
            if '=' in line:
                # Split the line into parts
                parts = line.strip().split('=')
                sensor_name = parts[0].strip()
                sensor_value = parts[1].strip().split()[0]

                # Append data to the lists
                sensor_names.append(sensor_name)
                sensor_values.append(sensor_value)

    # Create a DataFrame
    df = pd.DataFrame({
        'Sensor': sensor_names,
        'Value': sensor_values
    })

    # Convert 'Value' to a numeric type, handling non-numeric cases
    df['Value'] = pd.to_numeric(df['Value'], errors='coerce')

    return df


# Combine data from all files into a single DataFrame
files = ['sensor_data.txt', 'sensor_data1.txt', 'sensor_data2.txt', 'sensor_data3.txt','sensor_data4.txt', 'sensor_data5.txt', 'sensor_data6.txt']
all_data = pd.DataFrame()
for file in files:
    file_data = preprocess_data(file)
    all_data = pd.concat([all_data, file_data])

# Exploratory Data Analysis
# Plotting histograms for each sensor
for column in all_data.columns:
    plt.figure()
    sns.histplot(all_data[column], kde=True)
    plt.title(f'Distribution of {column}')
    plt.show()

# Correlation heatmap
plt.figure(figsize=(10, 8))
sns.heatmap(all_data.corr(), annot=True, cmap='coolwarm')
plt.title('Correlation Heatmap')
plt.show()

# Feature Engineering (if necessary)
# Example: all_data['new_feature'] = all_data['existing_feature'] ** 2

# Model Training
# Replace 'target_column' with the column you want to predict
X = all_data.drop('target_column', axis=1)  
y = all_data['target_column']
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

model = RandomForestRegressor(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

# Model Evaluation and Plotting Results
y_pred = model.predict(X_test)
print(f'Mean Squared Error: {mean_squared_error(y_test, y_pred)}')
print(f'R^2 Score: {r2_score(y_test, y_pred)}')

plt.figure()
plt.scatter(y_test, y_pred)
plt.xlabel('Actual Values')
plt.ylabel('Predicted Values')
plt.title('Actual vs Predicted')
plt.show()

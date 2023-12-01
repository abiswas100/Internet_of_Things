#https://lastminuteengineers.com/nrf24l01-arduino-wireless-communication/

import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_squared_error, r2_score

# Loading the dataset
file_path = 'dummy_sensor_data_5000.csv'
df = pd.read_csv(file_path)

# Prepare the data
X = df.drop('temperature', axis=1)
y = df['temperature']

# Split the data into training and testing sets (80-20 split)
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=0)

# Create and train the Random Forest Regressor
random_forest_regressor = RandomForestRegressor(n_estimators=100, random_state=0)
random_forest_regressor.fit(X_train, y_train)

# Make predictions on the test set
y_pred = random_forest_regressor.predict(X_test)

# Evaluate the model
mse = mean_squared_error(y_test, y_pred)
r2 = r2_score(y_test, y_pred)

# Print the evaluation metrics
print(f'Mean Squared Error (MSE): {mse}')
print(f'R-squared (R2): {r2}')


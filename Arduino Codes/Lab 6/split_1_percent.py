# Load all the libraries
import pandas as pd
import numpy as np
import pickle

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score, classification_report
from sklearn.linear_model import LogisticRegression

# Load the data

dataset = 'weatherAUS.csv'
rain = pd.read_csv(dataset)

# Reduce the cardinality of date by splitting it into year month and day
rain['Date'] = pd.to_datetime(rain['Date'])
rain['year'] = rain['Date'].dt.year
rain['month'] = rain['Date'].dt.month
rain['day'] = rain['Date'].dt.day
rain.drop('Date', axis = 1, inplace = True)

# Classify feature type
categorical_features = [
    column_name 
    for column_name in rain.columns 
    if rain[column_name].dtype == 'O'
]

numerical_features = [
    column_name 
    for column_name in rain.columns 
    if rain[column_name].dtype != 'O'
]

# Fill missing categorical values with the highest frequency value in the column
categorical_features_with_null = [
    feature 
    for feature in categorical_features 
    if rain[feature].isnull().sum()
]

for each_feature in categorical_features_with_null:
    mode_val = rain[each_feature].mode()[0]
    rain[each_feature].fillna(mode_val,inplace=True)

# Before treating the missing values in numerical values, treat the outliers
features_with_outliers = [
    'MinTemp', 
    'MaxTemp', 
    'Rainfall', 
    'Evaporation', 
    'WindGustSpeed',
    'WindSpeed9am', 
    'WindSpeed3pm', 
    'Humidity9am', 
    'Pressure9am', 
    'Pressure3pm', 
    'Temp9am', 
    'Temp3pm'
]

for feature in features_with_outliers:
    q1 = rain[feature].quantile(0.25)
    q3 = rain[feature].quantile(0.75)
    IQR = q3 - q1
    lower_limit = q1 - (IQR * 1.5)
    upper_limit = q3 + (IQR * 1.5)
    rain.loc[rain[feature]<lower_limit,feature] = lower_limit
    rain.loc[rain[feature]>upper_limit,feature] = upper_limit

# Treat missing values in numerical features
numerical_features_with_null = [
    feature 
    for feature in numerical_features 
    if rain[feature].isnull().sum()
]

for feature in numerical_features_with_null:
    mean_value = rain[feature].mean()
    rain[feature].fillna(mean_value,inplace=True)


# Encoding categorical values as integers
direction_encoding = {
    'W': 0, 'WNW': 1, 'WSW': 2, 'NE': 3, 'NNW': 4, 
    'N': 5, 'NNE': 6, 'SW': 7, 'ENE': 8, 'SSE': 9, 
    'S': 10, 'NW': 11, 'SE': 12, 'ESE': 13, 'E': 14, 'SSW': 15
}

location_encoding = {
    'Albury': 0, 
    'BadgerysCreek': 1, 
    'Cobar': 2, 
    'CoffsHarbour': 3, 
    'Moree': 4, 
    'Newcastle': 5, 
    'NorahHead': 6, 
    'NorfolkIsland': 7, 
    'Penrith': 8, 
    'Richmond': 9, 
    'Sydney': 10, 
    'SydneyAirport': 11, 
    'WaggaWagga': 12, 
    'Williamtown': 13, 
    'Wollongong': 14, 
    'Canberra': 15, 
    'Tuggeranong': 16, 
    'MountGinini': 17, 
    'Ballarat': 18, 
    'Bendigo': 19, 
    'Sale': 20, 
    'MelbourneAirport': 21, 
    'Melbourne': 22, 
    'Mildura': 23, 
    'Nhil': 24, 
    'Portland': 25,
    'Watsonia': 26, 
    'Dartmoor': 27, 
    'Brisbane': 28, 
    'Cairns': 29, 
    'GoldCoast': 30, 
    'Townsville': 31, 
    'Adelaide': 32, 
    'MountGambier': 33, 
    'Nuriootpa': 34, 
    'Woomera': 35, 
    'Albany': 36, 
    'Witchcliffe': 37, 
    'PearceRAAF': 38, 
    'PerthAirport': 39, 
    'Perth': 40, 
    'SalmonGums': 41, 
    'Walpole': 42, 
    'Hobart': 43, 
    'Launceston': 44, 
    'AliceSprings': 45, 
    'Darwin': 46, 
    'Katherine': 47, 
    'Uluru': 48
}
boolean_encoding = {'No': 0, 'Yes': 1}


rain['RainToday'].replace(boolean_encoding, inplace = True)
rain['RainTomorrow'].replace(boolean_encoding, inplace = True)
rain['WindGustDir'].replace(direction_encoding,inplace = True)
rain['WindDir9am'].replace(direction_encoding,inplace = True)
rain['WindDir3pm'].replace(direction_encoding,inplace = True)
rain['Location'].replace(location_encoding, inplace = True)

# See the distribution of the dataset
print(rain['RainTomorrow'].value_counts())

# Split features and target value as X and Y
X = rain.drop(['RainTomorrow'],axis=1)
y = rain['RainTomorrow']

# Split training and test split

# Xtrain_split and Y_train_split will contain the 99% of the dataset that will be split into train and test
# Xtest_split and Y_test_split contains the 1% of the dataset.

X_train_split_1, X_test_split_1, y_train_split_1, y_test_split_1 = train_test_split(X,y, test_size = 0.01, random_state = 0)

# Concatenate X_test_split_1 and y_test_split_1 along the column axis
combined_1_percent_data = pd.concat([X_test_split_1, y_test_split_1], axis=1)

# Save the combined DataFrame to a CSV file
combined_1_percent_data.to_csv('combined_test_data.csv', index=False)

# Drop the 1% test data from the original X and y
X_remaining = X.drop(X_test_split_1.index)
y_remaining = y.drop(y_test_split_1.index)

# Save the remaining X and y to separate CSV files
X_remaining.to_csv('X_remaining.csv', index=False)
y_remaining.to_csv('y_remaining.csv', index=False)


# Concatenate the remaining X and y along the column axis
combined_remaining_data = pd.concat([X_remaining, y_remaining], axis=1)

# Save the combined DataFrame to a CSV file
combined_remaining_data.to_csv('combined_remaining_data.csv', index=False)

import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
import matplotlib.pyplot as plt
import seaborn as sns
import os
import category_encoders as ce
from sklearn.tree import DecisionTreeRegressor
from sklearn.metrics import accuracy_score, mean_absolute_error
from sklearn.metrics import confusion_matrix

data_frame = pd.read_csv('samples/car_evaluation.csv', header=None)
print(data_frame.shape)

col_names = ['buying', 'maint', 'doors', 'persons', 'lug_boot', 'safety', 'class']


data_frame.columns = col_names
print(data_frame.head())

print(data_frame.describe())

for it in col_names:
    print(data_frame[it].value_counts())

# Input/Output split

X = data_frame.drop(columns='class',axis=1)
Y = data_frame['class']

X_train, X_test, Y_train, Y_test = train_test_split(X, Y, test_size=0.25, random_state=42)

print(X_train.shape, X_test.shape)

encoder_X = ce.OrdinalEncoder(cols=['buying', 'maint', 'doors', 'persons', 'lug_boot', 'safety'])
encoder_Y = ce.OrdinalEncoder(cols=['class'])
X_train = encoder_X.fit_transform(X_train)
X_test = encoder_X.transform(X_test)

Y_train = encoder_Y.fit_transform(Y_train)
Y_test = encoder_Y.transform(Y_test)


tree = DecisionTreeRegressor(criterion='absolute_error',random_state=42, min_samples_leaf=5)
tree.fit(X_train, Y_train)

Y_pred = tree.predict(X_test)
Y_pred_train = tree.predict(X_train)

Y_pred = [int(round(x)) for x in Y_pred]
Y_pred_train = [int(round(x)) for x in Y_pred_train]

print('Model accuracy score with criterion gini index: {0:0.4f}'. format(accuracy_score(Y_test, Y_pred)))
print('Train score {0:0.4f}'.format(accuracy_score(Y_train, Y_pred_train)))

cm = confusion_matrix(Y_test, Y_pred)
print('Confusion matrix\n\n', cm)

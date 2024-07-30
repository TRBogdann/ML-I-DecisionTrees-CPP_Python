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

data_frame = pd.read_csv('samples/diabetes_india.csv', header=None)
print(data_frame.shape)


# Define the array with the strings
columns = [
    "pregancies",
    "glucose",
    "pressure",
    "thickness",
    "insulin",
    "bmi",
    "pedigree",
    "age",
    "decision"
]


data_frame.columns = columns

X = data_frame.drop('decision', axis=1)
Y = data_frame['decision']

X_train, X_test, Y_train, Y_test = train_test_split(X, Y, test_size=0.25, random_state=42)

#{'squared_error', 'absolute_error', 'friedman_mse', 'poisson'}
tree = DecisionTreeRegressor(criterion='squared_error',random_state=42, min_samples_leaf=5)
tree.fit(X_train, Y_train)

Y_pred = tree.predict(X_test)
Y_pred_train = tree.predict(X_train)

Y_pred = [int(round(x)) for x in Y_pred]
Y_pred_train = [int(round(x)) for x in Y_pred_train]

print('Model accuracy score with criterion gini index: {0:0.4f}'. format(accuracy_score(Y_test, Y_pred)))
print('Train score {0:0.4f}'.format(accuracy_score(Y_train, Y_pred_train)))

cm = confusion_matrix(Y_test, Y_pred)
print('Confusion matrix\n\n', cm)
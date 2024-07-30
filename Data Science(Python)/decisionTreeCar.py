
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
import matplotlib.pyplot as plt
import seaborn as sns
import os
import category_encoders as ce
from sklearn.tree import DecisionTreeClassifier
from sklearn.metrics import accuracy_score
from sklearn.metrics import confusion_matrix
import joblib

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

encoder = ce.OrdinalEncoder(cols=['buying', 'maint', 'doors', 'persons', 'lug_boot', 'safety'])

X_train = encoder.fit_transform(X_train)
X_test = encoder.transform(X_test)

print(X_train.head())
print(X_test.head())

tree = DecisionTreeClassifier(random_state=42, min_samples_leaf=5)
tree.fit(X_train, Y_train)
print(tree.score(X_test, Y_test))

Y_pred = tree.predict(X_test)
Y_pred_train = tree.predict(X_train)
print('Model accuracy score with criterion gini index: {0:0.4f}'. format(accuracy_score(Y_test,Y_pred)))
print('Train score {0:0.4f}'.format(accuracy_score(Y_train,Y_pred_train)))

cm = confusion_matrix(Y_test, Y_pred)
print('Confusion matrix\n\n', cm)


joblib.dump(tree, 'model.pkl')
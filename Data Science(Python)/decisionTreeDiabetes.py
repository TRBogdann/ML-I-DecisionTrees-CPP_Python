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

df = pd.read_csv('samples/diabetes_prediction_dataset.csv')
print(df.shape)

for it in df.columns:
    print(df[it].value_counts())

X = df.drop(columns=['diabetes'], axis=1)
Y = df['diabetes']
X_train, X_test, Y_train, Y_test = train_test_split(X, Y, test_size=0.25, random_state=42)

encoder = ce.OrdinalEncoder(cols=['gender', 'smoking_history'])
X_train = encoder.fit_transform(X_train)
X_test = encoder.transform(X_test)

tree = DecisionTreeClassifier(criterion='gini', random_state=42,min_samples_leaf=100)
tree.fit(X_train, Y_train)

Y_pred = tree.predict(X_test)
Y_pred_train = tree.predict(X_train)

print('Model accuracy score with criterion gini index: {0:0.4f}'. format(accuracy_score(Y_test, Y_pred)))
print('Train score {0:0.4f}'.format(accuracy_score(Y_train, Y_pred_train)))

cm = confusion_matrix(Y_test, Y_pred)
print('Confusion matrix\n\n', cm)
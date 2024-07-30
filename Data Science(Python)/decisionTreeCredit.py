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
from scipy.io import arff


data_frame = pd.read_csv('samples/dataset_31_credit-g.csv', header=None)
print(data_frame.shape)

columns = [
    "status",
    "duration",
    "credit_history",
    "purpose",
    "credit",
    "savings",
    "employment",
    "installment_rate",
    "status_sex",
    "debtors",
    "resident_since",
    "property",
    "age",
    "installment_plans",
    "housing",
    "number_of_existing_credits",
    "job",
    "number_of_liable_people",
    "telephone",
    "local_or_foreign",
    "decision"
]

qualitative_columns = [
    "status",
    "credit_history",
    "purpose",
    "savings",
    "employment",
    "status_sex",
    "debtors",
    "property",
    "installment_plans",
    "housing",
    "job",
    "telephone",
    "local_or_foreign"
]

data_frame.columns = columns

X = data_frame.drop('decision', axis=1)
y = data_frame['decision']

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.25, random_state=42)

encoder = ce.OrdinalEncoder(cols=qualitative_columns)
X_train = encoder.fit_transform(X_train)
X_test = encoder.transform(X_test)

tree = DecisionTreeClassifier(criterion='gini', random_state=42,min_samples_leaf=10)
tree.fit(X_train, y_train)
y_pred = tree.predict(X_test)
y_pred_train = tree.predict(X_train)

print('Model accuracy score with criterion gini index: {0:0.4f}'. format(accuracy_score(y_test, y_pred)))
print('Train score {0:0.4f}'.format(accuracy_score(y_train, y_pred_train)))

cm = confusion_matrix(y_test, y_pred)
print('Confusion matrix\n\n', cm)
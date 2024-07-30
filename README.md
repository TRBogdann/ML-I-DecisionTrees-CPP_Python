# Machine-Learning-I-CPP_Python-

I compared my c++ models with the respective python solution to check wheter my solutions are correct or not.
The model had pretty close performances.While the C++ had better performances in some cases they also presented more signs of overfitting.

Problem 1 (Classifier)
Decision Tree Comparison for car evaluation:


C++ Model:
Test Acurracy:
98.0247
Train Accuracy:
99.4709
Confussion_Matrix:
Clasess: good acc vgood unacc
17 2 1 0
1 78 1 2
0 0 13 0
0 1 0 289

Python :
Test_Acuraccy:
87.27
Train_Accuracy:
93.36
Confussion matrix:
Clasess: acc,vgood,unacc,good
76   5  13   9
6   5   1   3
9   0 285   0
8   1   0  11

Problem 2

Decision Tree(Classifier) Comparison for Diabetes

C++ Model:
Model has an acurracy of 96.854
Your model doesn t present signs of overfitting(train accuracy<=test accuracy)

Confusion matrix:
Clasess: 1 0
1302 785
0 22865

Python Model:

Model accuracy score with criterion gini index: 0.9712
Train score 0.9723
Confusion matrix:
1449 701
18 22832

Problem 3

Decision Tree(Classifier) for Credit Score


C++ Model:

Model has an acurracy of 62.5532
The model s performance is fair, but it s not good enough. Consider tuning the model or using another model
Your model presents signs of overfitting(62.5532,72.549)

Confusion matrix:
Clasess: bad good
12 73
15 135

Python Model:

Model accuracy score with criterion gini index: 0.7640
Train score 0.8147
Confusion matrix

37  35
24 154


Problem 4

Regression Tree Car Evaluation

C++ Model:

Accuracy: 94.0741
The model presents signs of overfitting(94.0741,96.1451)
287 3 0 0
5 74 2 1
0 2 17 1
0 0 1 12

Python Model:

Model accuracy score with criterion gini index: 0.8657
Train score 0.8958
Confusion matrix
284 10 0 0
17 76 7 3
1 9 5 0
0 10 1 9

Problem 5
Regression Tree India

C++ Model:

Accuracy: 61.1429
The model presents signs of overfitting(61.1429,82.1248)
87 26
26 36

Python Model:

Model 2:
Model accuracy score with criterion gini index: 0.7031
Train score 0.8993
Confusion matrix
92 31
26 43

Model Choise
p1 C++>Python
p2 Python~C++(Similar performance)
p3 Python>C++
p4 C++>Python
p5 Python>C++

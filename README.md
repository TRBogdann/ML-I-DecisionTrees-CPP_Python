# Machine-Learning-I-CPP_Python-<br />

This repository covers the next topics:<br />
1.Decision Tree Classfier<br />
2.Decision Tree Regressor<br />
3.Random Forest (Beggining)<br />

I compared my c++ models with the respective python solution to check wheter my solutions are correct or not.<br />
The model had pretty close performances.While the C++ had better performances in some cases they also presented more signs of overfitting.<br />

Problem 1 (Classifier)<br />
Decision Tree Comparison for car evaluation:<br />

C++ Model:<br />
Test Acurracy:<br />
98.0247<br />
Train Accuracy:<br />
99.4709<br />
Confussion_Matrix:<br />
Clasess: good acc vgood unacc<br />
17 2 1 0<br />
1 78 1 2<br />
0 0 13 0<br />
0 1 0 289<br />

Python :<br />
Test_Acuraccy:<br />
87.27<br />
Train_Accuracy:<br />
93.36<br />
Confussion matrix:<br />
Clasess: acc,vgood,unacc,good<br />
76   5  13   9<br />
6   5   1   3<br />
9   0 285   0<br />
8   1   0  11<br />

Problem 2<br />

Decision Tree(Classifier) Comparison for Diabetes<br />

C++ Model:<br />
Model has an acurracy of 96.854<br />
Your model doesn t present signs of overfitting(train accuracy<=test accuracy)<br />

Confusion matrix:<br />
Clasess: 1 0<br />
1302 785<br />
0 22865<br />

Python Model:<br />

Model accuracy score with criterion gini index: 0.9712<br />
Train score 0.9723<br />
Confusion matrix:<br />
1449 701<br />
18 22832<br />

Problem 3<br />

Decision Tree(Classifier) for Credit Score<br />

C++ Model:<br />

Model has an acurracy of 62.5532<br />
The model s performance is fair, but it s not good enough. Consider tuning the model or using another model<br />
Your model presents signs of overfitting(62.5532,72.549)<br />

Confusion matrix:<br />
Clasess: bad good<br />
12 73<br />
15 135<br />

Python Model:<br />

Model accuracy score with criterion gini index: 0.7640<br />
Train score 0.8147<br />
Confusion matrix<br />

37  35<br />
24 154<br />

Problem 4<br />

Regression Tree Car Evaluation<br />

C++ Model:<br />

Accuracy: 94.0741<br />
The model presents signs of overfitting(94.0741,96.1451)<br />
287 3 0 0<br />
5 74 2 1<br />
0 2 17 1<br />
0 0 1 12<br />

Python Model:<br />

Model accuracy score with criterion gini index: 0.8657<br />
Train score 0.8958<br />
Confusion matrix<br />
284 10 0 0<br />
17 76 7 3<br />
1 9 5 0<br />
0 10 1 9<br />

Problem 5<br />
Regression Tree India<br />

C++ Model:<br />

Accuracy: 61.1429<br />
The model presents signs of overfitting(61.1429,82.1248)<br />
87 26<br />
26 36<br />

Python Model:<br />

Model 2:<br />
Model accuracy score with criterion gini index: 0.7031<br />
Train score 0.8993<br />
Confusion matrix<br />
92 31<br />
26 43<br />

Model Choise<br />
p1 C++>Python<br />
p2 Python~C++(Similar performance)<br />
p3 Python>C++<br />
p4 C++>Python<br />
p5 Python>C++<br />

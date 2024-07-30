#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <list>
#include <string>
#include <sstream>
#include <iostream>
#include <utility>

enum columnType { inputCol, outputCol };
enum columnDataType { numericData, atrib };

static inline void trim(std::string &s)
{
    if (isspace(s[s.length() - 1]))
        s.pop_back();
}

class DataException : public std::exception
{
public:
    DataException(std::string message) : std::exception(), message(message)
    {
    }

    const char *what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};

class NumericRow
{
public:
    NumericRow(int size, float *values)
    {
        this->size = size;
        if (values == NULL || size <= 0)
        {
            this->values = NULL;
            this->size = 0;
        }
        else
        {
            this->values = new float[size];
            for (int i = 0; i < size; i++)
            {
                this->values[i] = values[i];
            }
        };
    }

    NumericRow(const NumericRow &row)
    {
        this->size = row.size;
        if (row.values == NULL || row.size <= 0)
        {
            this->values = NULL;
            this->size = 0;
        }
        else
        {
            this->values = new float[size];
            for (int i = 0; i < size; i++)
            {
                this->values[i] = row.values[i];
            }
        };
    }

    float operator[](int index) const
    {
        if (index >= size)
        {
            throw DataException("DataRow Index out of bounds");
        }
        else
            return values[index];
    }
    std::string toString() const
    {
        std::string res = "";
        if (size > 0)
        {
            res += std::to_string(values[0]);
            for (int i = 1; i < size; i++)
            {
                res += "," + std::to_string(values[i]);
            }
        }
        return res;
    }

    ~NumericRow()
    {
        if (this->values != NULL)
        {
            delete[] values;
            values = NULL;
        }
    }

private:
    int size;
    float *values;
};

struct ColomnMeta
{
    std::string label;
    columnType type;
    columnDataType dataType;
    float begin;
    float end;
    int size;
    std::string *listOfValues;
};

class NumericDataSet
{
public:
    NumericDataSet(int columns, ColomnMeta *meta)
    {
        this->columns = columns;
        if (columns <= 0 || meta == NULL)
        {
            this->meta = NULL;
            this->columns = 0;
        }
        else
        {
            this->meta = new ColomnMeta[columns];
            for (int i = 0; i < columns; i++)
            {
                this->meta[i] = meta[i];
            }
        }
    }

    NumericDataSet(const NumericDataSet &set)
    {
        this->columns = set.columns;
        if (set.columns <= 0 || set.meta == NULL)
        {
            this->meta = NULL;
            this->columns = 0;
        }
        else
        {
            this->meta = new ColomnMeta[columns];
            for (int i = 0; i < columns; i++)
            {
                this->meta[i] = set.meta[i];
            }
        }

        for (auto it = set.rows.begin(); it != set.rows.end(); it++)
        {
            this->rows.push_front(*it);
        }
    }

    void addData(std::string filename)
    {
        std::ifstream f(filename);
        while (!f.eof())
        {
            std::string unformatted;
            std::getline(f, unformatted);
            std::istringstream tokener(unformatted);
            float *values = new float[columns];
            for (int i = 0; i < columns; i++)
            {
                bool choice = this->meta[i].dataType == numericData;
                std::string formatted;
                std::getline(tokener, formatted, ',');
                trim(formatted);
                if (choice)
                {
                    try
                    {
                        values[i] = std::stof(formatted);
                    }
                    catch (std::exception e)
                    {
                        std::cout << "Corrupted data";
                        values[i] = 0;
                    }
                }
                else
                {
                    int j = 0;
                    bool found = 0;
                    while (j < this->meta[i].size && !found)
                    {
                        if (formatted == meta[i].listOfValues[j])
                            found = 1;
                        else
                            j++;
                    }
                    if (j >= this->meta[i].size)
                    {
                        std::cout << "Corrupted data";
                        values[i] = 0;
                    }
                    else
                        values[i] = j;
                }
            }
            rows.push_front(NumericRow(columns, values));
            delete[] values;
        }
        f.close();
    };

    void show() const
    {
        for (int i = 0; i < columns; i++)
        {
            std::cout << meta[i].label << ",";
        }
        for (auto it = rows.begin(); it != rows.end(); it++)
        {
            std::cout << it->toString() << "\n";
        }
    }

    float getVariance(int outputCol) const
    {
        float variance = 0;
        float avg = 0;
        for (auto it = this->rows.begin(); it != this->rows.end(); it++)
        {
            avg += (*it)[outputCol];
        }
        avg /= rows.size();

        for (auto it = this->rows.begin(); it != this->rows.end(); it++)
        {
            float x = (*it)[outputCol] - avg;
            variance += x * x;
        }

        return variance / rows.size();
    }

    NumericDataSet **split(float med, int col_index) const
    {
        NumericDataSet **set = new NumericDataSet *[2];
        set[0] = new NumericDataSet(this->columns, this->meta);
        set[1] = new NumericDataSet(this->columns, this->meta);

        for (auto it = this->rows.begin(); it != this->rows.end(); it++)
        {
            if ((*it)[col_index] <= med)
            {
                set[0]->rows.push_front(NumericRow((*it)));
            }
            else
            {
                set[1]->rows.push_front(NumericRow((*it)));
            }
        }
        return set;
    };

    ~NumericDataSet()
    {
        if (meta != NULL)
        {
            delete[] meta;
            meta = NULL;
        }
    }

private:
    friend class RegressionTree;
    friend class RegressionNode;
    ColomnMeta *meta;
    std::list<NumericRow> rows;
    int columns;
};

class NumericSplit
{
public:
    ~NumericSplit()
    {
        if (intervals)
        {
            delete[] intervals;
            intervals = NULL;
        }
    }
    NumericSplit **split(int index, float med) const
    {
        NumericSplit **splits = new NumericSplit *[2];

        splits[0] = new NumericSplit();
        splits[1] = new NumericSplit();

        splits[0]->output = {output.first, output.second};
        splits[1]->output = {output.first, output.second};

        splits[0]->size = size;
        splits[1]->size = size;

        splits[0]->output_index = output_index;
        splits[1]->output_index = output_index;

        splits[0]->intervals = new std::pair<float, float>[size];
        splits[1]->intervals = new std::pair<float, float>[size];

        for (int i = 0; i < size; i++)
        {
            if (i != index)
            {
                splits[0]->intervals[i].first = this->intervals[i].first;
                splits[0]->intervals[i].second = this->intervals[i].second;
                splits[1]->intervals[i].first = this->intervals[i].first;
                splits[1]->intervals[i].second = this->intervals[i].second;
            }
            else
            {
                splits[0]->intervals[i].first = this->intervals[i].first;
                splits[0]->intervals[i].second = med;
                splits[1]->intervals[i].first = med;
                splits[1]->intervals[i].second = this->intervals[i].second;
            }
        }

        return splits;
    }
    std::pair<float, float> *intervals = NULL;
    int size = 0;
    std::pair<float, float> output;
    int output_index;
};

class RegressionNode
{
public:
    float getReduction(const NumericDataSet &set, int col_index, int output_index, float med, float &varianceL, float &varianceR)
    {
        float avgL = 0;
        float avgR = 0;
        float weightL = 0;
        float weightR = 0;
        varianceL = 0;
        varianceR = 0;

        for (auto it = set.rows.begin(); it != set.rows.end(); it++)
        {
            if ((*it)[col_index] <= med)
            {
                avgL += (*it)[output_index];
                weightL += 1;
            }
            else
            {
                avgR += (*it)[output_index];
                weightR += 1;
            }
        }

        if (weightL <= 1 || weightR <= 1)
            return -1;

        avgL /= weightL;
        avgR /= weightR;

        for (auto it = set.rows.begin(); it != set.rows.end(); it++)
        {
            if ((*it)[col_index] <= med)
            {
                varianceL += std::pow((*it)[output_index] - avgL, 2);
            }
            else
            {
                varianceR += std::pow((*it)[output_index] - avgR, 2);
            }
        }

        varianceL /= weightL;
        varianceR /= weightR;

        return -weightL * varianceL - weightR * varianceR;
    }

    RegressionNode(const NumericDataSet &set, int max_depth, int depth, int output_index) : left(NULL), right(NULL)
    {
        this->depth = depth;
        this->output_index = output_index;

        float variance = set.getVariance(output_index);

        if (variance == 0)
        {
            avg = (*set.rows.begin())[output_index];
            return;
        }
        else if (depth == max_depth)
        {
            float avg = 0;
            for (auto it = set.rows.begin(); it != set.rows.end(); it++)
            {
                avg += (*it)[output_index];
            }

            this->avg = avg / set.rows.size();
            return;
        }
        else
        {
            float best_reduction = 0;
            int best_col = -1;
            float best_med = 0;
            float best_varianceL = 0;
            float best_varianceR = 0;

            for (int i = 0; i < set.columns; i++)
            {
                if (set.meta[i].type == outputCol)
                    continue;

                float max = set.meta[i].end;
                float min = set.meta[i].begin;
                int size = set.meta[i].size;
                float step = (max - min) / size;
                float varianceL, varianceR;

                for (float j = min; j < max; j += step)
                {
                    float reduction = getReduction(set, i, output_index, j, varianceL, varianceR);
                    if (reduction > best_reduction)
                    {
                        best_reduction = reduction;
                        best_col = i;
                        best_med = j;
                        best_varianceL = varianceL;
                        best_varianceR = varianceR;
                    }
                }
            }

            if (best_reduction > 0)
            {
                this->index = best_col;
                this->value = best_med;

                NumericDataSet **subsets = set.split(best_med, best_col);

                left = new RegressionNode(*subsets[0], max_depth, depth + 1, output_index);
                right = new RegressionNode(*subsets[1], max_depth, depth + 1, output_index);

                delete subsets[0];
                delete subsets[1];
                delete[] subsets;
            }
            else
            {
                float avg = 0;
                for (auto it = set.rows.begin(); it != set.rows.end(); it++)
                {
                    avg += (*it)[output_index];
                }

                this->avg = avg / set.rows.size();
                return;
            }
        }
    }

    ~RegressionNode()
    {
        if (left)
        {
            delete left;
            left = NULL;
        }
        if (right)
        {
            delete right;
            right = NULL;
        }
    }

    float getAvg() const
    {
        if (!left || !right)
            return avg;
        else
        {
            if (this->left && !this->right)
                return this->left->getAvg();
            if (!this->left && this->right)
                return this->right->getAvg();
        }
        return 0;
    }

    RegressionNode *getLeft() const
    {
        return left;
    }

    RegressionNode *getRight() const
    {
        return right;
    }

    float getValue() const
    {
        return value;
    }

    int getIndex() const
    {
        return index;
    }

private:
    RegressionNode *left;
    RegressionNode *right;
    float value;
    float avg;
    int index;
    int depth;
    int output_index;
};

class RegressionTree
{
public:
    RegressionTree(NumericDataSet set, int max_depth, int output_index) : set(set), max_depth(max_depth), output_index(output_index)
    {
        root = new RegressionNode(set, max_depth, 0, output_index);
    }

    ~RegressionTree()
    {
        if (root)
        {
            delete root;
            root = NULL;
        }
    }

    void train()
    {
        if (root)
        {
            delete root;
            root = NULL;
        }
        root = new RegressionNode(set, max_depth, 0, output_index);
    }

    float predict(const NumericRow &row) const
    {
        RegressionNode *node = root;
        while (node->getLeft() && node->getRight())
        {
            if (row[node->getIndex()] <= node->getValue())
                node = node->getLeft();
            else
                node = node->getRight();
        }
        return node->getAvg();
    }

private:
    RegressionNode *root;
    NumericDataSet set;
    int max_depth;
    int output_index;
};

int main()
{
    std::string list1[] = {"vhigh", "high", "med", "low"};
    std::string list2[] = {"vhigh", "high", "med", "low"};
    std::string list3[] = {"2", "3", "4", "5more"};
    std::string list4[] = {"2", "4", "more"};
    std::string list5[] = {"small", "med", "big"};
    std::string list6[] = {"low", "med", "high"};
    std::string outputList[] = {"unacc", "acc", "good", "vgood"};

    ColomnMeta metas[] =
        {
            {"price", inputCol, atrib, 0, 3, 4, list1},
            {"maintenance", inputCol, atrib, 0, 3, 4, list2},
            {"doors", inputCol, atrib, 0, 3, 4, list3},
            {"people", inputCol, atrib, 0, 2, 3, list4},
            {"luggage", inputCol, atrib, 0, 2, 3, list5},
            {"safety", inputCol, atrib, 0, 2, 3, list6},
            {"decision", outputCol, atrib, 0, 3, 4, outputList}};

    NumericDataSet dataSet(7, metas);
    dataSet.addData("car_evaluation.csv");
    RegressionTree tree(dataSet, 4, 0);
    tree.train();

    return 0;
}

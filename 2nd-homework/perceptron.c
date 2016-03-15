#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITERATIONS 500
#define LEARNING_RATE 0.01
#define MAX_MISCLASSIFIED 0
#define LABEL -1



int GetData(char* filename, int ***features, int *files_no, int *features_no, int *clean_no, int *mal_no)
{
    FILE *fd;
    char *line = NULL;
    char *splited;
    size_t len = 0;
    ssize_t read;
    int i, j;

    fd = fopen(filename, "r");   

    if (fd == NULL)
        return 1;

    while ((read = getline(&line, &len, fd)) != -1) 
    {
        *files_no += 1;
        splited = strchr(line, '|');

        if (splited == NULL)
            continue;

        if (*(splited + 1) == 'M')
            *mal_no += 1;
        else 
            if (*(splited + 1) == 'C')
                *clean_no += 1;

        if ((*files_no) == 1)
        {
            splited = strtok(line, "|");
            while (splited != NULL)
            {
                *features_no += 1;
                splited = strtok(NULL, "|");
            }
        }
    }

    *features_no -= 2;

    printf("files = %d, features = %d, malwares = %d, clean = %d.", *files_no, *features_no, *mal_no, *clean_no);

    *features = (int**)calloc(*files_no, sizeof(int*));

    if (*features == NULL)
        return 1;

    for (i = 0; i < *files_no; i++)
    {
        (*features)[i] = (int*)calloc((*features_no) + 1, sizeof(int));

        if ((*features)[i] == NULL)
            return 1;
    }

    fseek(fd, 0, SEEK_SET);

    i = 0;
    while ((read = getline(&line, &len, fd)) != -1) 
    {
        j = 0;
        
        splited = strtok(line, "|");
        while (splited != NULL)
        {
            splited = strtok(NULL, "|");

            if (splited == NULL)
                break;

            if (splited[0] == 'M')
                (*features)[i][j++] = 1;
            else if (splited[0] == 'C')
                (*features)[i][j++] = -1;
            else
                (*features)[i][j++] = atoi(splited);
        }   

        i++;
    }

    fclose(fd);

    return 0;
}


int PutData(char *filename, double *weights, int features_no)
{
    FILE *fd;
    int i;

    fd = fopen(filename, "w");   

    if (fd == NULL)
        return 1;

    for (i = 0; i < features_no - 1; i++)
        fprintf(fd, "%f|", weights[i]);

    fprintf(fd, "%f", weights[features_no-1]);

    return 0;
}


double GetSumForRecordAndWeights(int *record_features, double *weights, int features_no)
{
    int i;
    double features_sum = 0;

    for (i = 0; i < features_no; i++)
    {   
        features_sum = features_sum + *(record_features + 1 + i) * weights[i];
    }

    return features_sum;
}


int IsCorrectlyClassified(int label, int *record_features, double *weights, double bias, int features_no)
{
    double sum = GetSumForRecordAndWeights(record_features, weights, features_no);
    sum += bias;

    if (((label * sum) + 0.00) <= 0)
        return 0;

    return 1;
}


void Train(int **features, double *weights, double *bias, int *misclassified, double learning_rate, int features_no, int class_no)
{
    int i, j;
    double record_sum;
    double delta_bias = 0;
    double delta_weights[features_no];

    for (i = 0; i < features_no; i++)
        delta_weights[i] = 0;

    for (i = 0; i < class_no; i++)
    {
        if (IsCorrectlyClassified(features[i][0], features[i], weights, *bias, features_no) == 0)
        {
            for (j = 1; j <= features_no; j++)
                delta_weights[j-1] = delta_weights[j-1] + features[i][0] * learning_rate * features[i][j];
            delta_bias = delta_bias + features[i][0] * learning_rate;
            *misclassified = 1;
        }
    }

    for (i = 0; i < features_no; i++)
        weights[i] += delta_weights[i];

    *bias += delta_bias;
}


void GetSpecFeaturesVector(int **features, int **spec_class_features, int label, int files_no, int features_no)
{
    int i, j, k = 0;

    for (i = 0; i < files_no; i++)
        if (features[i][0] == label)
        {
            for (j = 0; j <= features_no; j++)
            {
                spec_class_features[k][j] = features[i][j];
            }

            k++;
        }
}


int OneSidePerceptron(int **features, double *weights, double *best_weights, double *best_bias, int max_iterations, double learning_rate, double max_misclassified, int label, int features_no, int files_no, int spec_class_no)
{
    int i;
    int errors;
    int misclassified;
    int iteration = 0;
    double bias = 0;
    int **spec_class_features;

    int min_miscalssified = files_no;

    for (i = 0; i < features_no; i++)
        weights[i] = 0;

    spec_class_features = (int**)calloc(spec_class_no, sizeof(int*));

    if (spec_class_features == NULL)
        return 1;

    for (i = 0; i < spec_class_no; i++)
    {
        spec_class_features[i] = (int*)calloc(features_no + 1, sizeof(int));

        if (spec_class_features[i] == NULL)
            return 1;
    }

    GetSpecFeaturesVector(features, spec_class_features, label, files_no, features_no);
    do 
    {
        printf("\n------------------------------");
        printf("\nIteration: %d", iteration);
        
        misclassified = 0;
        Train(features, weights, &bias, &misclassified, learning_rate, features_no, files_no);

        errors = 0;
        for (i = 0; i < files_no; i++)
            if (IsCorrectlyClassified(features[i][0], features[i], weights, bias, features_no) == 0)
            {
                errors += 1;
            }
        printf("\nAll errors: %d", errors);

        do
        {
            Train(spec_class_features, weights, &bias, &misclassified, learning_rate, features_no, spec_class_no);
            errors = 0;
            for (i = 0; i < spec_class_no; i++)
            {
                if (IsCorrectlyClassified(spec_class_features[i][0], spec_class_features[i], weights, bias, features_no) == 0)
                {
                    errors += 1;
                }
            }
            printf("\nErrors: %d", errors);
        } while (errors > max_misclassified);

        errors = 0;
        for (i = 0; i < files_no; i++)
            if (IsCorrectlyClassified(features[i][0], features[i], weights, bias, features_no) == 0)
            {
                errors += 1;
            }

        printf("\nAll errors after spec class classifing: %d with bias: %f", errors, bias);

        if (errors <= min_miscalssified)
        {
            min_miscalssified = errors;
            *best_bias = bias;
            for (i = 0; i < features_no; i++)
                best_weights[i] = weights[i];
        }

        printf("\nBest until now: %d with bias: %f", min_miscalssified, *best_bias);
        printf("\n------------------------------\n");

        iteration++;
    } while (iteration < max_iterations && misclassified == 1);

    printf("\n\n\nBEST: %d\n\n\n", min_miscalssified);

    for (i = 0; i < spec_class_no; i++)
        free(spec_class_features[i]);

    free(spec_class_features);

    return 0;
}

int main(int argc, char* argv[])
{
    int features_no = 0, files_no = 0, clean_no = 0, mal_no = 0;
    int **features;
    double *weights, *best_weights;
    double best_bias;
    int res;
    int i;
    int a = -1;

    if (argc < 2)
    {
        printf("Usage: %s <features_files>", argv[0]);
        exit(1);
    }

    res = GetData(argv[1], &features, &files_no, &features_no, &clean_no, &mal_no);

    if (res == 1)
    {
        printf("Error!");
        return 1;
    }

    weights = (double*)calloc(features_no, sizeof(double));

    if (weights == NULL)
    {
        printf("Error!");
        return 1;
    }

    best_weights = (double*)calloc(features_no, sizeof(double));

    if (best_weights == NULL)
    {
        printf("Error!");
        return 1;
    }

    res = OneSidePerceptron(features, weights, best_weights, &best_bias, MAX_ITERATIONS, LEARNING_RATE, MAX_MISCLASSIFIED, LABEL, features_no, files_no, clean_no);

    if (res == 1)
    {
        printf("Error!");
        return 1;
    }
    printf("BEST_BIAS = %f", best_bias);

    res = PutData("weights", best_weights, features_no);

    if (res == 1)
    {
        printf("Error!");
        return 1;
    }

    for (i = 0; i < files_no; i++)
    {
        free(features[i]);
    }

    free(features);
    free(weights);
    free(best_weights);

    return 0;
}

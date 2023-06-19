# Description
Mondrian is a Top-down greedy data anonymization algorithm for relational dataset,
proposed by Kristen LeFevre in his papers [\[1\]](#more-information).
To our knowledge, Mondrian is the fastest local recording algorithm,
which preserve good data utility at the same time. Although LeFevre gave
the pseudocode in his papers, the original source code is not available.
You can find the third part Java implementation in
Anonymization Toolbox [\[2\]](#more-information).

This repository is an **open source C implementation for Mondrian**.

**Contents:**
  - [Motivation](#motivation)
  - [Attention](#attention)
  - [Basic idea of Mondrian](#basic-idea-of-mondrian)
    - [First, what is k-anonymity?](#first-what-is-k-anonymity)
    - [How Mondrian anonymizes dataset?](#how-mondrian-anonymizes-dataset)
  - [Usage](#usage)
    - [Requirements](#requirements)
    - [Compilation and execution](#compilation-and-execution)
  - [References](#references)
  - [Support](#support)
  - [Acknowledgements](#acknowledgements)
  - [License](#license)

# Motivation
Researches on data privacy have lasted for more than ten years, lot of
great papers have been published. However, only a few open source projects
are available on Internet [\[2-3\]](#more-information), most open source
projects are using algorithms proposed before 2004! Fewer projects have been
used in real life. Worse more, most people even don't hear about it. Such a tragedy!

I decided to make some effort. Hoping these open source repositories can help
researchers and developers on data privacy (privacy preserving data publishing,
data anonymization).

# Attention
This Mondrian is the earliest Mondrian proposed in [\[1\]](#more-information),
which imposes an intuitive ordering on each attribute. So, there is no generalization
hierarchies for categorical attributes. This operation brings lower information loss,
but worse semantic results. **If you want the Mondrian based on generalization
hierarchies, please turn to [Basic_Mondrian](https://github.com/qiyuangong/Basic_Mondrian).**

I used **both adult and INFORMS** dataset in this implementation.
For clarification, **we transform NCP (Normalized Certainty Penalty) to percentage**.
This NCP percentage is computed by dividing NCP value with the number of values in
dataset (also called GCP (Global Certainty Penalty) [\[4\]](#more-information)).
The range of NCP percentage is from 0 to 1, where 0 means no information loss,
1 means loses all information (more meaningful than raw NCP, which is sensitive
to size of dataset).

One more thing!!! Mondrian has strict and relax models. (Most online implementations
are in strict model.) Both Mondrian split partition with binary split (let lhs and
rhs denotes left part and right part). In strict Mondrian, lhs has not intersection
part with rhs. But in relaxed Mondrian, the points in the middle are evenly divided
between lhs and rhs to ensure `|lhs| = |rhs|` (+1 where `|partition|` is odd). So
in relax model, the generalized result of lhs and rhs may have intersection.

The Final NCP of Mondrian on [ADULT dataset](https://archive.ics.uci.edu/ml/datasets/adult)
is about 24.91% (relax) and 12.19% (strict), while 12.26% (relax) and 10.21% (strict)
on [INFORMS dataset](https://sites.google.com/site/informsdataminingcontest/) (with K=10).

# Basic idea of Mondrian
## First, what is k-anonymity?
Assuming your record is in this format: [QID, SA]. QID means quasi-identifier
such as age and birthday, SA means sensitive information such as disease information.
The basic idea of k-anonymity is `safety in group` (or safety in numbers [\[5\]](#more-information)),
which means that you are safe if you are in a group of people whose QIDs are the same.
Note nobody can infer your sensitive information (SA) from this group using QID,
as shown in Fig. 1 (k=3 in 1(b) and 1(c)). If each of these group has at least k people,
then this dataset satisfy k-anonymity.

<div align="center">
    <img src=https://cloud.githubusercontent.com/assets/3848789/25949050/c6a7e8ec-3688-11e7-933d-d5a991e6ef30.png width=750>
    <p align="center">Figure 1. Anonymity, Privacy and Generalization</p>
</div>


**But in practice, the raw datasets usually don't satisfy k-anonymity,
as shown in Fig. 1(a).** So, we need some help from anonymization algorithm to
transform the raw datasets to anonymized datasets. Mondrian is one of them, and it
is based on generalization. I don't want to talk too much about generalization.
In a word, generalization is a kind of transformation, which finds a result QID*
that covers all QIDs (QID1~QID3 in Fig. 1 (b)). And it also brings information loss (distortion).

## How Mondrian anonymizes dataset?
Here is the basic workflow of Mondrian:

1. Partition the raw dataset into k-groups using kd-tree. k-groups means that each
group contains at least k records.
2. Generalization each k-group (Fig. 1(b)), such that each group has the same QID*.

Why using kd-tree? Because it is fast, straight-forward and sufficient.

<div align="center">
    <img src=https://cloud.githubusercontent.com/assets/3848789/25949051/c6a87622-3688-11e7-8bd0-726f07245570.png width=750>
    <p align="center">Figure 2. Basic workflow of Modnrian</p>
</div>

<div align="center">
    <img src=https://cloud.githubusercontent.com/assets/3848789/25949052/c6ab3fce-3688-11e7-99ea-cde7bccd8684.png width=450>
    <p align="center">Figure 3. kd-tree</p>
</div>

# Usage
## Requirements
- cmake
- build-essential

## Compilation and execution
The implementation is based on [Qiyuan Gong](https://github.com/qiyuangong/Mondrian)
python implementation. You can run Mondrian in following steps:

1. Download (or clone) the whole project.
2. Create a directory named `build`. Change the working directory to it.
3. Run the command
   ```bash
   $ cmake ..
   ```
   If you want debug symbols run
   ```bash
   $ cmake -DCMAKE_BUILD_TYPE=Debug ..
   ```
4. Run `mondrian`
   ```bash
   $ ./mondrian -h
   Usage:
        -f DATASET               Dataset file path. Default: ../datasets/adults.csv
        -o OUTPUT                Output file path. Default: output.csv
        -m strict|relaxed        Mondrian mode. Default: strict
        -a                       If present, anonymize output attributes.
        -r                       If present, only generate results (no output file).
   ```
   > The dataset should have the following format:<br>
   > (Row #1): Name of the attributes<br>
   > (Row #2): Quasi identifier indexes<br>
   > (Rest of the rows): Value of the attributes<br>
   > Example:
   >
   > ```csv
   > age, work_class, final_weight, education, education_num, marital_status, occupation, relationship, race, sex, capital_gain, capital_loss, hours_per_week, native_country, class
   > 0, 1, 4, 5, 6, 8, 9, 13
   > 39, State-gov, 77516, Bachelors, 13, Never-married, Adm-clerical, Not-in-family, White, Male, 2174, 0, 40, United-States, <=50K
   > 50, Self-emp-not-inc, 83311, Bachelors, 13, Married-civ-spouse, Exec-managerial, Husband, White, Male, 0, 0, 13, United-States, <=50K
   > 38, Private, 215646, HS-grad, 9, Divorced, Handlers-cleaners, Not-in-family, White, Male, 0, 0, 40, United-States, <=50K
   > 53, Private, 234721, 11th, 7, Married-civ-spouse, Handlers-cleaners, Husband, Black, Male, 0, 0, 40, United-States, <=50K
   > 28, Private, 338409, Bachelors, 13, Married-civ-spouse, Prof-specialty, Wife, Black, Female, 0, 0, 40, Cuba, <=50K
   > 37, Private, 284582, Masters, 14, Married-civ-spouse, Exec-managerial, Wife, White, Female, 0, 0, 40, United-States, <=50K
   > 49, Private, 160187, 9th, 5, Married-spouse-absent, Other-service, Not-in-family, Black, Female, 0, 0, 16, Jamaica, <=50K
   > 52, Self-emp-not-inc, 209642, HS-grad, 9, Married-civ-spouse, Exec-managerial, Husband, White, Male, 0, 0, 45, United-States, >50K
   > ```

> A script to simplify the build, execution and testing have been added. Check ./run.sh -h for more information

# References
1. K. LeFevre, D. J. DeWitt, R. Ramakrishnan. Mondrian Multidimensional K-Anonymity
ICDE '06: Proceedings of the 22nd International Conference on Data Engineering,
IEEE Computer Society, 2006, 25
2. [UTD Anonymization Toolbox](http://cs.utdallas.edu/dspl/cgi-bin/toolbox/index.php?go=home)
3. [ARX- Powerful Data Anonymization](https://github.com/arx-deidentifier/arx)
4. G. Ghinita, P. Karras, P. Kalnis, N. Mamoulis. Fast data anonymization with low
information loss. Proceedings of the 33rd international conference on Very large
Data Bases, VLDB Endowment, 2007, 758-769
5. Y. He, J. F. Naughton, Anonymization of set-valued data via top-down,
local generalization. Proceedings of VLDB, 2009, 2, 934-945

# Support
- You can post bug reports and feature requests at the [Issue Page](https://github.com/scmanjarrez/Mondrian/issues).
- Contributions via [Pull request](https://github.com/scmanjarrez/Mondrian/pulls) is welcome.

# Acknowledgements
**This work has been supported by __TBD__ and co-financed by __TBD__**

# License
```
Mondrian  Copyright (C) 2023 Sergio Chica @ csic.es.
Consejo Superior de Investigaciones Científicas.
This program comes with ABSOLUTELY NO WARRANTY; for details check below.
This is free software, and you are welcome to redistribute it
under certain conditions; check below for details.
```

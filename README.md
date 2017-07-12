nao-sound-classification
========================

The sound classification module provides the following functionality:

1. A machine learning module, based on the OpenCV implementation of the random trees algorithm, for training a sound classifier from a labeled set of recordings.

2. A classifier that can process a batch prerecorded audio files and output the resulting confusion matrix.

3. A naoqi module for online sound classification, using Nao's microphones as inputs.

"Remote" use (on the development PC)
====================================

Installation
-------------

Although this use-case technically does not require naoqi, due to legacy reasons it also uses the naoqi build system. It requires naoqi v2.1.

Dependencies
------------

For reading/writing audio files:
```
sudo apt install libsndfile1-dev
```

For preprocessing audio files:
```
sudo apt install sox
```

For getting audio file properties in Python scripts:
```
sudo pip install soundfile
```

For extracting audio features:
```
git clone https://github.com/jamiebullock/LibXtract
cd LibXtract
sudo make install PREFIX=/usr/local
```

Building the software
---------------------

Clone this git repo into your qiworkspace. Make sure to select the host toolchain (SDK). The PC_SIDE needs to be set to ON in order to build a package for remote use.
```
cd nao-sound-classification
qibuild configure
qibuild make
```

For convenience, add the path to the utility scripts to your `PATH` environment variable, to have them accessible from any folder (append the above command to your `~/.bashrc` file):
```
export PATH=$PATH:<path to this repo>/scripts
```

Workflow
--------

Although technically not necessary, it is currently most convenient to create a `Resources` folder inside `<build folder>/sdk/bin` and copy the dataset there in a folder named Dataset (this layout is currently assumed by the default configuration files and utility scripts).

1. Prepare the audio files, i.e. convert them to 16kHz Mono format (parts of the feature extarctor are hardcoded to these values).
     
     ```
     cd <build folder>/sdk/bin/Resources
     mkdir Dataset_mono_16k
     prepare_audio.sh
     ```
 The `prepare_audio.sh` script expects to be invoked from the folder where the dataset resides, it expects it to be in the `Dataset` subfolder and it requires the `Dataset_mono_16k` to have been created beforehand.

2. Create a list of files in the dataset and their corresponding labels 

     ```
     create_dataset_list.py Dataset_mono_16k
     ```
 This will create `output.csv` and `output.txt` files, both files containing a list of files in the dataset with their coresponing labels, in different formats. The `output.txt` is currently used by C++ tools.

3. Generate a summary of the dataset contents (number of samples and total duration for each class). Requires the list of dataset files and the folder where the dataset is located
     
     ```
     transform_dataset_list.py output.txt Dataset_mono_16k
     ```
 This script optionally allows dataset relabeling.

4. Split the dataset into training and test sampls

     ```
     split_train_test_data.py output.txt class_info.txt
     ```
 This will generate `test_data.txt` and `train_data.txt`.
 
5. Create and customize the classifier configuration file

    ```
    cd .. # Go back to the bin folder where the executables are located
    ./Configurator SC.config
    ```
 The resulting `SC.config` configuration file is a line-oriented plain text file, which does not support comments. The layout must be as follows:
 
    0. Sound library folder (where sound samples are located)
    
    1. List of files for the training dataset (output of the `split_train_test_data.py` script)
    
    2. Feature selection string (used by Learner, fileClassify and folderClassify), each letter represents on feature to be used for classification. The feature `k` is currently broken due to a bug in LibXtract and should not be used; Best results are currently obtained by using the `lop` features
    
    3. Sound features data (csv table generated by learner). Each row corrensponds to one chunk, columns correspond to feature values (several columns can belog to one feature - some features are vectors), only features selected by the feature selection string are present, always ordered as in the feature selection string
    
    4. Learned classes list. The machine learning model encodes classes as numbers; this file (generated by the learner) provides the mapping between the numbers and the class names; class listed in the first row is encoded with 0, class in the second row is 1 etc 
    
    5. The classification model. An xml file generated by machine learning (Learner app), contains the OpenCV data structure encoding the machine learning model
    
    6. Robot IP for connecting to the robot when classifying a live data stream
    
    7. Robot port for connecting to the robot when classifying a live data stream
    
    8. Sound sample length [pow(2, x)] - each recording is chopped up into pieces consisting of  2^(sound sample length) samples 
    
    9. subSample length [pow(2, x)] - each piece is chopped up into smaller piecesconsisting of 2^(subSample length) samples; currently used only by HZCRR (high zero crossing rate ratio) 

6. Learn the model (random trees)
     
     ```
     ./Learner SC.config
     ```
 If everything works correctly, this will generate the `model.xml` file.

7. Validate the model

     ```
     ./FolderClassify SC.config
     ```
Line 2. in `SC.config` names the dataset that will be used for validation. By default, this is the training data. To use test data, modify this line in `SC.config`.

8. In order to carry out cross-validation, run the `cross_validation.py` script. You can run cross-validation for Train or Test dataset. (Just replace the word when calling the script). Additional two parameters (i,j) can be passed to the script in order to plot the moving average value of element (i,j) of the confusion matrices:
    ```
    cross-validation test 1 1
    ```


"Local" use (on the robot)
==========================


1. Set the PC_SIDE inside `CMakeLists.txt` to OFF

2. Compile the LibXtract library on the OpenNAO software by running it on the virtual machine, copy it to your computer and run add it to the cross-toolchain:

    ```
    qitoolchain add-package -c nao-cross <path to the created zip file>
    ```

3. Compile the package by running:

    ```
    qibuild configure -c nao-cross
    qibuild make -c nao-cross
    ```

4. Copy the created library file `libSCModule.so` from <path to cross-toolchain build folder>/sdk/lib/naoqi/ to the robot inside /home/nao/naoqi/modules/ folder by running the command on your computer:

    ```
    scp <file path> nao@192.168.1.105:~/naoqi/modules/
    ```

5. Add this line to the autoload.ini file inside /home/nao/naoqi/modules under the [user] line:

    ```
    /home/nao/naoqi/modules/libSCModule.so
    ```

6. Copy `model.xml`, `classList.txt` and `SC.config` files inside Resources folder on the robot. Make sure that the full file path is used inside `SC.config`.

7. Copy `naoClass` executable binary file form <path to cross-toolchain build folder>/sdk/bin/naoqi/ on the robot using scp as before.

8. After restarting the robot you can check whether the SCModule is running on NAO robot page

9. Run the classification on the robot by running:

    ```
    ./naoClass /home/nao/<path to Resources folder>/SC.config
    ```

**Note:** You can also run the classification remotely  (naoClassify executable in the remote module build folder)

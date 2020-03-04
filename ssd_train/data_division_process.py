from data_preparation.preparation_utils import data_division
import os

data_path = './data'
annotations_path = os.path.join(data_path, 'annotations.csv')

percentages = {'train': 70, 'val': 10, 'test': 5, 'train_fine_tuning':5, 'val_fine_tuning':5, 'test_fine_tuning':5}

shuffling_info = {'INCLUDE': True, 'random_seed': 1000, 'num_shuffles': 5}

'''
Samples taken for each task are exported in the given annotation_path, and the user can access to each one 
through the <datasets> object's attributes.' 

they are accecible in datasets object whithin the attributes below (in dataframe format):
    
    train             : datasets.train_data
    validation        : datasets.val_data
    test              : datasets.test_data
    train fine-tuning : datasets.train_fine_tuning_data
    val fine-tuning   : datasets.val_fine_tuning_data
    test fine-tuning  : datasets.test_fine_tuning_data
    
'''
datasets  = data_division(annotations_path, percentages, shuffling_info)
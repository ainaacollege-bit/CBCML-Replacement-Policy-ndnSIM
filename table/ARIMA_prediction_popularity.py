#!/usr/bin/env python
# coding: utf-8

# In[1]:


import pandas as pd
import numpy as np
from statsmodels.tsa.arima.model import ARIMA
from sklearn.metrics import mean_absolute_error, mean_squared_error
import matplotlib.pyplot as plt

pd.options.mode.chained_assignment = None  # default='warn'


# In[2]:


# Load the data from the uploaded CSV file
df = pd.read_csv('content-cache-trace.csv')
#df['Timestamp'] = df['Timestamp'] * 1000
df['Timestamp'] = df['Timestamp'].astype(int)

# Only keep 'hit' events since they represent requests
df_hits = df[df['Event'] == 'hit']

# Convert Timestamp to a datetime format for resampling
#df_hits['Timestamp'] = pd.to_datetime(df_hits['Timestamp'], unit='s', origin='unix')

# Convert seconds to timedelta and create a TimeIndex  
df_hits['time_index'] = pd.to_timedelta(df_hits['Timestamp'], unit='s')  
df_hits['Timestamp'] = pd.to_datetime('1970-01-01') + df_hits['time_index']  
print(df_hits.dtypes)


# In[3]:


print(df_hits)


# In[4]:


# Group by ContentID and resample based on fixed time intervals (e.g., 30-second intervals)
def resample_content_requests(content_df, interval=5):
    content_df = content_df.set_index('Timestamp')  # Set timestamp as index
    resampled = content_df.resample(f'{interval}S').size()  # Count hits in each interval
    return resampled.fillna(0)  # Fill gaps with 0s (no requests in those intervals)


# In[5]:


# Define a function to resample, train, forecast, and evaluate for each content ID
def process_content_forecast(content_id, group, interval=120, forecast_steps=5):
    # Resample the data for regular intervals (e.g., 120 seconds) and fill missing intervals with 0s
    group = resample_content_requests(group, interval=5)
    #group = group.set_index('Timestamp').resample(f'{interval}S').size().fillna(0)
    
    # Check if there's enough data for ARIMA
    if len(group) <= forecast_steps:
        print(f"Insufficient data for Content ID {content_id}. Using mean-based forecast as fallback.")
        fallback_forecast = pd.Series([group.mean()] * forecast_steps, index=pd.date_range(start=group.index[-1], periods=forecast_steps+1, freq=f'{interval}S')[1:])
        return fallback_forecast.sum(), None, None, None, None  # Fallback to mean-based forecast
    
    # Split data into train and test sets
    train, test = group[:-forecast_steps], group[-forecast_steps:]
    
    # Train ARIMA model on the training set
    try:
        model = ARIMA(train, order=(5, 1, 0))
        model_fit = model.fit()
        
        # Forecast over the test period
        forecast = model_fit.forecast(steps=forecast_steps)
        
        # Calculate evaluation metrics
        mae = mean_absolute_error(test, forecast)
        mse = mean_squared_error(test, forecast)
        rmse = np.sqrt(mse)
        mape = np.mean(np.abs((test - forecast) / test)) * 100
        
        # Plot the historical data, forecast, and test data
        plt.figure(figsize=(10, 5))
        plt.plot(train.index, train, label='Training Data')
        plt.plot(test.index, test, label='Test Data', color='blue')
        plt.plot(test.index, forecast, label='Forecasted Data', color='red')
        plt.title(f'Content ID: {content_id} - Actual vs. Forecasted Requests')
        plt.legend()
        plt.show()
        
        return forecast.sum(), mae, mse, rmse, mape  # Sum of forecast as popularity score
    
    except Exception as e:
        print(f"ARIMA model failed for Content ID {content_id}: {e}")
        return None, None, None, None, None  # Ensure consistent return values in case of error


# In[6]:


# Dictionary to store predicted popularity and evaluation metrics by Content ID
predicted_popularity = {}
evaluation_metrics = {}

# Process each ContentID
for content_id, group in df_hits.groupby('ContentID'):
    print(f"\nProcessing Content ID: {content_id}")
    popularity_score, mae, mse, rmse, mape = process_content_forecast(content_id, group, interval=120, forecast_steps=5)
    
    if popularity_score is not None:
        predicted_popularity[content_id] = popularity_score
        evaluation_metrics[content_id] = {
            'MAE': mae,
            'MSE': mse,
            'RMSE': rmse,
            'MAPE': mape
        }


# In[7]:


# Display the predicted popularity and evaluation metrics
print("\nPredicted Popularity and Evaluation Metrics for Each Content ID:")
for content_id, score in predicted_popularity.items():
    metrics = evaluation_metrics[content_id]
    print(f"Content ID: {content_id}")
    print(f"  Predicted Popularity (Next 5 Intervals): {score:.2f}")
    if metrics['MAE'] is not None:
        print(f"  MAE: {metrics['MAE']:.2f}, MSE: {metrics['MSE']:.2f}, RMSE: {metrics['RMSE']:.2f}, MAPE: {metrics['MAPE']:.2f}%")
    else:
        print("  Metrics not available due to insufficient data (used fallback forecast).")


# In[8]:


# Save predicted popularity scores to a CSV file
predicted_popularity_df = pd.DataFrame.from_dict(predicted_popularity, orient='index', columns=['Predicted_Popularity'])
predicted_popularity_df.to_csv('predicted_popularity.csv')


# In[ ]:





# In[ ]:





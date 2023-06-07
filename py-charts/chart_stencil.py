import matplotlib.pyplot as plt
import pandas as pd
import numpy as np


try: 
  filename = input("\nHello, enter .txt file name for chart generation: ")
  data = np.loadtxt(filename)
  
  # Extract the data into separate lists for easier plotting
  time_overall = data[:, 0]
  cpu_time = data[:, 1]
  num_processes = data[:, 2]
  rows = data[:, 3]
  columns = data[:, 4]
  num_iterations = data[:, 5]

  # Extract the data into sub-categories
  time_overall_matrix1 = time_overall[0:5]
  time_overall_matrix2 = time_overall[5:10]
  time_overall_matrix3 = time_overall[10:15]
  time_overall_matrix4 = time_overall[15:20]
  time_overall_matrix5 = time_overall[20:25]
  
  cpu_times_matrix1 = cpu_time[0:5]
  cpu_times_matrix2 = cpu_time[5:10]
  cpu_times_matrix3 = cpu_time[10:15]
  cpu_times_matrix4 = cpu_time[15:20]
  cpu_times_matrix5 = cpu_time[20:25]
  
  matrix_sizes = [int(rows[0]), int(rows[5]), int(rows[10]), int(rows[15]), int(rows[20])]
  
  process_numbers = [int(num_processes[0]), int(num_processes[1]), int(num_processes[2]), int(num_processes[3]), int(num_processes[4])]
  
  # Calculate Overall Speedup 
  speedup_overall_matrix1 = []
  speedup_overall_matrix2 = []
  speedup_overall_matrix3 = []
  speedup_overall_matrix4 = []
  speedup_overall_matrix5 = []
  for i in range(len(time_overall_matrix1)):
    if (i == 0):
      speedup_overall_matrix1.append(1)
      speedup_overall_matrix2.append(1)
      speedup_overall_matrix3.append(1)
      speedup_overall_matrix4.append(1)
      speedup_overall_matrix5.append(1)
    else:
      speedup_overall_matrix1.append(time_overall_matrix1[0] / time_overall_matrix1[i])
      speedup_overall_matrix2.append(time_overall_matrix2[0] / time_overall_matrix2[i])
      speedup_overall_matrix3.append(time_overall_matrix3[0] / time_overall_matrix3[i])
      speedup_overall_matrix4.append(time_overall_matrix4[0] / time_overall_matrix4[i])
      speedup_overall_matrix5.append(time_overall_matrix5[0] / time_overall_matrix5[i])

  # Calculate CPU Speedup 
  cpu_speedup_matrix1 = []
  cpu_speedup_matrix2 = []
  cpu_speedup_matrix3 = []
  cpu_speedup_matrix4 = []
  cpu_speedup_matrix5 = []
  for i in range(len(cpu_times_matrix1)):
    if (i == 0):
      cpu_speedup_matrix1.append(1)
      cpu_speedup_matrix2.append(1)
      cpu_speedup_matrix3.append(1)
      cpu_speedup_matrix4.append(1)
      cpu_speedup_matrix5.append(1)
    else:
      cpu_speedup_matrix1.append(cpu_times_matrix1[0] / cpu_times_matrix1[i])
      cpu_speedup_matrix2.append(cpu_times_matrix2[0] / cpu_times_matrix2[i])
      cpu_speedup_matrix3.append(cpu_times_matrix3[0] / cpu_times_matrix3[i])
      cpu_speedup_matrix4.append(cpu_times_matrix4[0] / cpu_times_matrix4[i])
      cpu_speedup_matrix5.append(cpu_times_matrix5[0] / cpu_times_matrix5[i])

  # Calculate Overall Efficiency 
  overall_efficiency_matrix1 = []
  overall_efficiency_matrix2 = []
  overall_efficiency_matrix3 = []
  overall_efficiency_matrix4 = []
  overall_efficiency_matrix5 = []
  for i in range(len(speedup_overall_matrix1)):
    if (i == 0):
      overall_efficiency_matrix1.append(1)
      overall_efficiency_matrix2.append(1)
      overall_efficiency_matrix3.append(1)
      overall_efficiency_matrix4.append(1)
      overall_efficiency_matrix5.append(1)
    else:
      overall_efficiency_matrix1.append(speedup_overall_matrix1[i] / process_numbers[i])
      overall_efficiency_matrix2.append(speedup_overall_matrix2[i] / process_numbers[i])
      overall_efficiency_matrix3.append(speedup_overall_matrix3[i] / process_numbers[i])
      overall_efficiency_matrix4.append(speedup_overall_matrix4[i] / process_numbers[i])
      overall_efficiency_matrix5.append(speedup_overall_matrix5[i] / process_numbers[i])

  # Calculate CPU Efficeincy
  cpu_efficiency_matrix1 = []
  cpu_efficiency_matrix2 = []
  cpu_efficiency_matrix3 = []
  cpu_efficiency_matrix4 = []
  cpu_efficiency_matrix5 = []
  for i in range(len(cpu_speedup_matrix1)):
    if (i == 0):
      cpu_efficiency_matrix1.append(1)
      cpu_efficiency_matrix2.append(1)
      cpu_efficiency_matrix3.append(1)
      cpu_efficiency_matrix4.append(1)
      cpu_efficiency_matrix5.append(1)
    else:
      cpu_efficiency_matrix1.append(cpu_speedup_matrix1[i] / process_numbers[i])
      cpu_efficiency_matrix2.append(cpu_speedup_matrix2[i] / process_numbers[i])
      cpu_efficiency_matrix3.append(cpu_speedup_matrix3[i] / process_numbers[i])
      cpu_efficiency_matrix4.append(cpu_speedup_matrix4[i] / process_numbers[i])
      cpu_efficiency_matrix5.append(cpu_speedup_matrix5[i] / process_numbers[i])
          
  # @Create plots of Time_Overall vs. Num_Processes
  plt.plot(time_overall_matrix1, marker='o')
  plt.plot(time_overall_matrix2, marker='o')
  plt.plot(time_overall_matrix3, marker='o')
  plt.plot(time_overall_matrix4, marker='o')
  plt.plot(time_overall_matrix5, marker='o')
  # Define table attributes
  tick_locs = np.linspace(0, 4, 5)
  plt.xticks(tick_locs, [*process_numbers])
  plt.title("Overall time vs. Number of processes")
  plt.xlabel("Number of processes")
  plt.ylabel("Overall Time in seconds")
  plt.xlim(tick_locs[0], tick_locs[-1])
  plt.legend([*matrix_sizes], loc='upper right')
  # Table data
  col_labels = [*process_numbers]
  dff = pd.DataFrame(col_labels)
  row_labels = [*matrix_sizes]
  table_vals = [["%.3f"%item for item in time_overall_matrix1], 
                ["%.3f"%item for item in time_overall_matrix2], 
                ["%.3f"%item for item in time_overall_matrix3], 
                ["%.3f"%item for item in time_overall_matrix4], 
                ["%.3f"%item for item in time_overall_matrix5]]
  row_colors = ['blue', 'orange', 'green', 'red', 'purple']
  # Plot table
  table = plt.table(cellText=table_vals,
                     colWidths=[0.1] * 5,
                     rowLabels=row_labels,
                     colLabels=col_labels,
                     rowColours=row_colors,
                     cellLoc = 'center', 
                     rowLoc = 'center',
                     loc='bottom',
                     bbox=[0.25, -0.5, 0.5, 0.3]
                     )
  table.auto_set_column_width(col=list(range(len(dff.columns)))) 
  # Adjust layout to make room for the table
  plt.subplots_adjust(left=0.2, bottom=.3)
  # Format the table
  table.auto_set_font_size(False)
  table.set_fontsize(7.5)
  table.scale(1, 1.5)
  # Save
  plt.savefig('overall_times.png', bbox_inches='tight')
  plt.clf()


  # @Create plots of CPU_Time vs. Num_Processes
  plt.plot(cpu_times_matrix1, marker='o')
  plt.plot(cpu_times_matrix2, marker='o')
  plt.plot(cpu_times_matrix3, marker='o')
  plt.plot(cpu_times_matrix4, marker='o')
  plt.plot(cpu_times_matrix5, marker='o')
  # Format chart
  tick_locs = np.linspace(0, 4, 5)
  plt.xticks(tick_locs, [*process_numbers])
  plt.title("CPU time vs. Number of processes")
  plt.xlabel("Number of processes")
  plt.ylabel("CPU Time in seconds")
  plt.xlim(tick_locs[0], tick_locs[-1])
  plt.legend([*matrix_sizes], loc='upper right')
  # Table data
  col_labels = [*process_numbers]
  dff = pd.DataFrame(col_labels)
  row_labels = [*matrix_sizes]
  table_vals = [["%.3f"%item for item in cpu_times_matrix1], 
                ["%.3f"%item for item in cpu_times_matrix2], 
                ["%.3f"%item for item in cpu_times_matrix3], 
                ["%.3f"%item for item in cpu_times_matrix4], 
                ["%.3f"%item for item in cpu_times_matrix5]]
  row_colors = ['blue', 'orange', 'green', 'red', 'purple']
  # Plot table
  table = plt.table(cellText=table_vals,
                     colWidths=[0.1] * 5,
                     rowLabels=row_labels,
                     colLabels=col_labels,
                     rowColours=row_colors,
                     cellLoc = 'center', 
                     rowLoc = 'center',
                     loc='bottom',
                     bbox=[0.25, -0.5, 0.5, 0.3]
                     )
  table.auto_set_column_width(col=list(range(len(dff.columns)))) 
  # Adjust layout to make room for the table
  plt.subplots_adjust(left=0.2, bottom=.3)
  # Format the table
  table.auto_set_font_size(False)
  table.set_fontsize(7)
  table.scale(1, 1.5)
  # Save
  plt.savefig('cpu_times.png', bbox_inches='tight')
  plt.clf()
  
  
  # @Create plots of Overall Speedup
  plt.plot(speedup_overall_matrix1, marker='o')
  plt.plot(speedup_overall_matrix2, marker='o')
  plt.plot(speedup_overall_matrix3, marker='o')
  plt.plot(speedup_overall_matrix4, marker='o')
  plt.plot(speedup_overall_matrix5, marker='o')
  # Define table attributes
  tick_locs = np.linspace(0, 4, 5)
  plt.xticks(tick_locs, [*process_numbers])
  plt.title("Overall Speedup")
  plt.xlabel("Number of processes")
  plt.ylabel("Speedup in Times Faster")
  plt.xlim(tick_locs[0], tick_locs[-1])
  plt.legend([*matrix_sizes], loc='upper left')
  # Table data
  col_labels = [*process_numbers]
  dff = pd.DataFrame(col_labels)
  row_labels = [*matrix_sizes]
  table_vals = [["%.3f"%item for item in speedup_overall_matrix1], 
                ["%.3f"%item for item in speedup_overall_matrix2], 
                ["%.3f"%item for item in speedup_overall_matrix3], 
                ["%.3f"%item for item in speedup_overall_matrix4], 
                ["%.3f"%item for item in speedup_overall_matrix5]]
  row_colors = ['blue', 'orange', 'green', 'red', 'purple']
  # Plot table
  table = plt.table(cellText=table_vals,
                     colWidths=[0.1] * 5,
                     rowLabels=row_labels,
                     colLabels=col_labels,
                     rowColours=row_colors,
                     cellLoc = 'center', 
                     rowLoc = 'center',
                     loc='bottom',
                     bbox=[0.25, -0.5, 0.5, 0.3]
                     )
  table.auto_set_column_width(col=list(range(len(dff.columns)))) 
  # Adjust layout to make room for the table
  plt.subplots_adjust(left=0.2, bottom=.3)
  # Format the table
  table.auto_set_font_size(False)
  table.set_fontsize(7)
  table.scale(1, 1.5)
  # Save
  plt.savefig('overall_speedup.png', bbox_inches='tight')
  plt.clf()
  
  
  # @Create plots of CPU Speedup
  plt.plot(cpu_speedup_matrix1, marker='o')
  plt.plot(cpu_speedup_matrix2, marker='o')
  plt.plot(cpu_speedup_matrix3, marker='o')
  plt.plot(cpu_speedup_matrix4, marker='o')
  plt.plot(cpu_speedup_matrix5, marker='o')
  # Define table attributes
  tick_locs = np.linspace(0, 4, 5)
  plt.xticks(tick_locs, [*process_numbers])
  plt.title("CPU Speedup")
  plt.xlabel("Number of processes")
  plt.ylabel("CPU Speedup in Times Faster")
  plt.xlim(tick_locs[0], tick_locs[-1])
  plt.legend([*matrix_sizes], loc='upper left')
  # Table data
  col_labels = [*process_numbers]
  dff = pd.DataFrame(col_labels)
  row_labels = [*matrix_sizes]
  table_vals = [["%.3f"%item for item in cpu_speedup_matrix1], 
                ["%.3f"%item for item in cpu_speedup_matrix2], 
                ["%.3f"%item for item in cpu_speedup_matrix3], 
                ["%.3f"%item for item in cpu_speedup_matrix4], 
                ["%.3f"%item for item in cpu_speedup_matrix5]]
  row_colors = ['blue', 'orange', 'green', 'red', 'purple']
  # Plot table
  table = plt.table(cellText=table_vals,
                     colWidths=[0.1] * 5,
                     rowLabels=row_labels,
                     colLabels=col_labels,
                     rowColours=row_colors,
                     cellLoc = 'center', 
                     rowLoc = 'center',
                     loc='bottom',
                     bbox=[0.25, -0.5, 0.5, 0.3]
                     )
  table.auto_set_column_width(col=list(range(len(dff.columns)))) 
  # Adjust layout to make room for the table
  plt.subplots_adjust(left=0.2, bottom=.3)
  # Format the table
  table.auto_set_font_size(False)
  table.set_fontsize(7)
  table.scale(1, 1.5)
  # Save
  plt.savefig('cpu_speedup.png', bbox_inches='tight')
  plt.clf()
  
  
  # @Create plots of Overall Efficiency
  plt.plot(overall_efficiency_matrix1, marker='o')
  plt.plot(overall_efficiency_matrix2, marker='o')
  plt.plot(overall_efficiency_matrix3, marker='o')
  plt.plot(overall_efficiency_matrix4, marker='o')
  plt.plot(overall_efficiency_matrix5, marker='o')
  # Define table attributes
  tick_locs = np.linspace(0, 4, 5)
  plt.xticks(tick_locs, [*process_numbers])
  plt.title("Overall Efficiency")
  plt.xlabel("Number of processes")
  plt.ylabel("Percentage of Processor Utilization")
  plt.xlim(tick_locs[0], tick_locs[-1])
  plt.legend([*matrix_sizes], loc='lower left')
  # Table data
  col_labels = [*process_numbers]
  dff = pd.DataFrame(col_labels)
  row_labels = [*matrix_sizes]
  table_vals = [["%.3f"%item for item in overall_efficiency_matrix1], 
                ["%.3f"%item for item in overall_efficiency_matrix2], 
                ["%.3f"%item for item in overall_efficiency_matrix3], 
                ["%.3f"%item for item in overall_efficiency_matrix4], 
                ["%.3f"%item for item in overall_efficiency_matrix5]]
  row_colors = ['blue', 'orange', 'green', 'red', 'purple']
  # Plot table
  table = plt.table(cellText=table_vals,
                     colWidths=[0.1] * 5,
                     rowLabels=row_labels,
                     colLabels=col_labels,
                     rowColours=row_colors,
                     cellLoc = 'center', 
                     rowLoc = 'center',
                     loc='bottom',
                     bbox=[0.25, -0.5, 0.5, 0.3]
                     )
  table.auto_set_column_width(col=list(range(len(dff.columns)))) 
  # Adjust layout to make room for the table
  plt.subplots_adjust(left=0.2, bottom=.3)
  # Format the table
  table.auto_set_font_size(False)
  table.set_fontsize(7)
  table.scale(1, 1.5)
  # Save
  plt.savefig('overall_efficiency.png', bbox_inches='tight')
  plt.clf()
  
  
  # @Create plots of CPU Efficiency
  plt.plot(cpu_efficiency_matrix1, marker='o')
  plt.plot(cpu_efficiency_matrix2, marker='o')
  plt.plot(cpu_efficiency_matrix3, marker='o')
  plt.plot(cpu_efficiency_matrix4, marker='o')
  plt.plot(cpu_efficiency_matrix5, marker='o')
  # Define table attributes
  tick_locs = np.linspace(0, 4, 5)
  plt.xticks(tick_locs, [*process_numbers])
  plt.title("CPU Efficiency")
  plt.xlabel("Number of processes")
  plt.ylabel("Percentage of Processor Utilization")
  plt.xlim(tick_locs[0], tick_locs[-1])
  plt.legend([*matrix_sizes], loc='lower left')
  # Table data
  col_labels = [*process_numbers]
  dff = pd.DataFrame(col_labels)
  row_labels = [*matrix_sizes]
  table_vals = [["%.3f"%item for item in cpu_efficiency_matrix1], 
                ["%.3f"%item for item in cpu_efficiency_matrix2], 
                ["%.3f"%item for item in cpu_efficiency_matrix3], 
                ["%.3f"%item for item in cpu_efficiency_matrix4], 
                ["%.3f"%item for item in cpu_efficiency_matrix5]]
  row_colors = ['blue', 'orange', 'green', 'red', 'purple']
  # Plot table
  table = plt.table(cellText=table_vals,
                     colWidths=[0.1] * 5,
                     rowLabels=row_labels,
                     colLabels=col_labels,
                     rowColours=row_colors,
                     cellLoc = 'center', 
                     rowLoc = 'center',
                     loc='bottom',
                     bbox=[0.25, -0.5, 0.5, 0.3]
                     )
  table.auto_set_column_width(col=list(range(len(dff.columns)))) 
  # Adjust layout to make room for the table
  plt.subplots_adjust(left=0.2, bottom=.3)
  # Format table
  table.auto_set_font_size(False)
  table.set_fontsize(7)
  table.scale(1, 1.5)
  # Save
  plt.savefig('cpu_efficiency.png', bbox_inches='tight')
  plt.clf()
  
  print("Thanks, your charts have been generated!")

except FileNotFoundError:
    print("Error: The specified file could not be found.")
except IOError:
    print("Error: An error occurred while reading the file.")
except Exception as e:
    print("Error: An unexpected error occurred: ", e)

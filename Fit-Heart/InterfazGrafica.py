import tkinter as tk
from tkinter import ttk
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import time
import threading
import socket

################################################################
#                                                              #
#              FUNCIÓN DE COMUNICAIÓN UDP                      #
#                                                              #
################################################################

host = '0.0.0.0'
port = 4444
buffer_size = 200
sock_data = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_data.bind((host, port))

# Función que lee datos por medio de protocolo UDP
def recolectar_datos():
    csv_data = sock_data.recv(buffer_size).decode()
    csv_data_parsed = csv_data.split(',')
    pulso = int(csv_data_parsed[0])
    bpm = round(float(csv_data_parsed[1]), 2)
    hrv = round(float(csv_data_parsed[2]), 2)
    temp = round(float(csv_data_parsed[3]), 2)
    intensidad = int(csv_data_parsed[4])
    return pulso, temp, bpm, hrv, intensidad

################################################################
#                                                              #
#              SE CREA LA FUNCIÓN LECTORA DE DATOS             #
#                                                              #
################################################################

# Función para leer datos en segundo plano
def read_data_thread():
    global dataPulso, bpm, hrv, intensidad, dataBPM, dataHRV, temp, intensidad
    while True:
        try:
            pulso, Newtemp, Newbpm, Newhrv, Newintensidad = recolectar_datos()
            if pulso is not None:
                if (Newbpm != 0):
                    # Se adjuntan todos los datos obtenidos
                    dataBPM.append(Newbpm)
                    bpm = Newbpm
                    if len(dataBPM) > 60:
                        dataBPM.pop(0)

                    temp = Newtemp + 9

                    intensidad = Newintensidad

                if (Newhrv != 0):
                    # Se adjunta el HRV medido en el último minuto
                    dataHRV.append(Newhrv)
                    hrv = Newhrv
                    if len(dataHRV) > 60:
                        dataHRV.pop(0)

                # Se actualiza el pulso siempre
                dataPulso.append(pulso)
                if len(dataPulso) > 50:
                    dataPulso.pop(0)                  

        except Exception as e:
            print(f"Error en la lectura de datos: {e}")
        time.sleep(0.04)  # Ajusta este valor para controlar la velocidad de lectura

################################################################
#                                                              #
#              DEFINICIÓN DE VARIABLES GLOBALES                #
#                                                              #
################################################################

# Se crean las variables para el pulso y se usa un hilo para leerlas
dataPulso = []  # Lista para almacenar los datos
data_thread = threading.Thread(target=read_data_thread)
data_thread.daemon = True
data_thread.start()
umbral_pulso = 1700

# Se crean las variables para BPM
dataBPM = []
bpm = 0

# Se crean las variables para HRV
hrv = 0
dataHRV = []

# Se crean las variables para temperatura
temp = 0

# Se crean las variables para la intensidad del ejercicio
intensidad = 0
colores = ["#006400", "#008000", "#00FF00", "#FFFF00", "#FFA500", "#FF0000"]

################################################################
#                                                              #
#              FUNCIONES QUE ACTUALIZAN GRÁFICAS               #
#                                                              #
################################################################

# Función para actualizar la gráfica de pulso
def update_graph_pulso():
    global umbral_pulso, dataPulso
    ax.clear()
    ax.plot(dataPulso, color='b', label='Pulso')

    # Línea punteada en y=umbral
    ax.axhline(y=umbral_pulso, linestyle='--', color='red', label='Umbral de latido')

    # ax.set_ylim(500, 600)
    ax.set_xlabel('Tiempo')
    ax.set_xticklabels([])
    ax.set_ylabel('Pulso')
    ax.set_title("Pulso en el tiempo")
    canvas.draw()
    root.after(50, update_graph_pulso)  # Ajusta la velocidad de actualización

# Función para actualizar la gráfica de bpm/hrv y barra de intensidad
def update_graph_bpm_hrv():
    global dataBPM, dataHRV, colores, intensidad, etiquetas

    ax_bpm.clear()
    
    # Asigna un color a cada valor de BPM
    colors = []
    for bpm in dataBPM:
        if bpm < 60:
            colors.append('blue')
        elif 60 <= bpm <= 85:
            colors.append('green')
        elif 85 < bpm <= 120:
            colors.append('yellow')
        else:
            colors.append('red')

    ax_bpm.bar(range(len(dataBPM)), dataBPM, color=colors)
    ax_bpm.set_xlabel('Tiempo')
    ax_bpm.set_ylim(0, 220)
    ax_bpm.set_ylabel('BPM')
    ax_bpm.set_xticklabels([])
    ax_bpm.set_title("BPM Recientes")

    ax_hrv.clear()
    ax_hrv.plot(range(len(dataHRV)), dataHRV, color='green')
    ax_hrv.set_xlabel('Tiempo')
    ax_hrv.set_ylabel('HRV (ms)', color='green')
    ax_hrv.set_ylim(0, 200)
    ax_hrv.set_xticks([])
    ax_hrv.set_title("HRV (RMSSD) Recientes")

    canvas_bpm.draw()
    canvas_hrv.draw()

    # Se actualiza la barra de progreso
    for i, etiqueta in enumerate(etiquetas):
        if (i < intensidad):
            etiqueta.configure(bg=colores[i], width=16, height=2)
        else:
            etiqueta.configure(bg="white", width=16, height=2)

    root.after(1000, update_graph_bpm_hrv)  # Ajusta la velocidad de actualización

################################################################
#                                                              #
#     FUNCIÓN QUE ACTUALIZA VALORES EN ETIQUETAS Y DATAS       #
#                                                              #
################################################################

# Función para actualizar los valores de BPM
def actualizar_bpm_hrv_temp():
    global bpm, hrv, temp
    
    # Actualiza las etiquetas con los nuevos valores de BPM y temperatura
    bpm_value_label.config(text=str(bpm))
    temperatura_value_label.config(text=str(temp))
    hrv_value_label.config(text=str(hrv))
    
    # Programa la actualización periódica (cada 1 segundos)
    root.after(1000, actualizar_bpm_hrv_temp)

# Se crea un botón de reinicio
def reiniciar_datos():
    global data, dataBPM, dataHRV, intensidad
    data = []
    dataBPM = []
    dataHRV = []
    intensidad = 0

################################################################
#                                                              #
#           SE CREA LA ESTRUCTURA DE LA INTERFAZ               #
#                                                              #
################################################################

# Inicializar ventana
root = tk.Tk()
root.title("Monitor de Salud")

# Crear frames
frame_pulsos = ttk.LabelFrame(root, text="Monitorio de Variables")
frame_pulsos.grid(row=1, column=0, padx=10, pady=10, sticky="ew")

frame_estres = ttk.LabelFrame(root, text="Nivel de intensidad de ejercicio")
frame_estres.grid(row=2, column=0, padx=10, pady=10, sticky="ew")

# Se crean etiquetas en Datos de Salud
frame_datos = ttk.Frame(frame_pulsos, borderwidth=0, relief="flat")
frame_datos.grid(row=0, column=0, padx=10, pady=10, sticky="nsew")

bpm_label = ttk.Label(frame_datos, text="BPM: ", font=("Arial", 12))
bpm_label.grid(row=0, column=0, padx=5, pady=5, sticky="w")
bpm_value_label = ttk.Label(frame_datos, text="", font=("Arial", 12))
bpm_value_label.grid(row=0, column=1, padx=5, pady=5, sticky="w")

hrv_label = ttk.Label(frame_datos, text="HRV (RMSSD): ", font=("Arial", 12))
hrv_label.grid(row=1, column=0, padx=5, pady=5, sticky="w")
hrv_value_label = ttk.Label(frame_datos, text="", font=("Arial", 12))
hrv_value_label.grid(row=1, column=1, padx=5, pady=5, sticky="w")

temperatura_label = ttk.Label(frame_datos, text="Temperatura: ", font=("Arial", 12))
temperatura_label.grid(row=2, column=0, padx=5, pady=5, sticky="w")
temperatura_value_label = ttk.Label(frame_datos, text="", font=("Arial", 12))
temperatura_value_label.grid(row=2, column=1, padx=5, pady=5, sticky="w")

reinicio_button = ttk.Button(frame_datos, text="REINICIO", command=reiniciar_datos)
reinicio_button.grid(row=3, column=0, columnspan=2, pady=10, sticky="w")

################################################################
#                                                              #
#     SE CREAN LAS GRÁFICAS Y SE MANDAN A ACTUALIZAR           #
#                                                              #
################################################################

# Se crea la gráfica en Monitorio de Pulsos
fig = Figure(figsize=(6, 2.5), dpi=100)
ax = fig.add_subplot(111)
ax.set_xlabel('Tiempo')
ax.set_ylabel('Pulso')
canvas = FigureCanvasTkAgg(fig, master=frame_pulsos)
canvas.get_tk_widget().grid(row = 0, column = 1, padx = 2, pady = 2)
update_graph_pulso()

# Se crea la gráfica para BPM
fig_bpm = Figure(figsize=(6, 2.5), dpi=100)
ax_bpm = fig_bpm.add_subplot(111)
ax_bpm.set_xlabel('Tiempo')
ax_bpm.set_ylabel('BPM')
canvas_bpm = FigureCanvasTkAgg(fig_bpm, master=frame_pulsos)
canvas_bpm.get_tk_widget().grid(row = 1, column = 0, padx = 2, pady = 2)

# Se crea la gráfica para HRV
fig_hrv = Figure(figsize=(6, 2.5), dpi=100)
ax_hrv = fig_hrv.add_subplot(111)
ax_hrv.set_xlabel('Tiempo')
ax_hrv.set_ylabel('HRV')
canvas_hrv = FigureCanvasTkAgg(fig_hrv, master=frame_pulsos)
canvas_hrv.get_tk_widget().grid(row = 1, column = 1, padx = 2, pady = 2)

# Se crean barra de intensidad en Nivel de intensidad
# Se crean las etiquetas dentro del LabelFrame
etiquetaZ1 = tk.Label(frame_estres, text="Z1", font=("Arial", 14, "bold"))
etiquetaZ2 = tk.Label(frame_estres, text="Z2", font=("Arial", 14, "bold"))
etiquetaZ3 = tk.Label(frame_estres, text="Z3", font=("Arial", 14, "bold"))
etiquetaZ4 = tk.Label(frame_estres, text="Z4", font=("Arial", 14, "bold"))
etiquetaZ5 = tk.Label(frame_estres, text="Z5", font=("Arial", 14, "bold"))
etiquetaZ6 = tk.Label(frame_estres, text="Z6", font=("Arial", 14, "bold"))
# Se colocan las etiquetas en el LabelFrame usando grid
etiquetaZ1.grid(row=0, column=0)
etiquetaZ2.grid(row=0, column=1)
etiquetaZ3.grid(row=0, column=2)
etiquetaZ4.grid(row=0, column=3)
etiquetaZ5.grid(row=0, column=4)
etiquetaZ6.grid(row=0, column=5)
# Se asignan colores de fondo a las etiquetas
etiquetas = [etiquetaZ1, etiquetaZ2, etiquetaZ3, etiquetaZ4, etiquetaZ5, etiquetaZ6]
etiquetas[0].configure(bg="#FFFFFF", width=16, height=2)
etiquetas[1].configure(bg="#FFFFFF", width=16, height=2)
etiquetas[2].configure(bg="#FFFFFF", width=16, height=2)
etiquetas[3].configure(bg="#FFFFFF", width=16, height=2)
etiquetas[4].configure(bg="#FFFFFF", width=16, height=2)
etiquetas[5].configure(bg="#FFFFFF", width=16, height=2)

# Se mandan a actualizar las datas
update_graph_bpm_hrv()
# Se mandan a actualizar las gráficas y la barra
actualizar_bpm_hrv_temp()

################################################################
#                                                              #
#           SE INICIALIZA LA INTERFAZ GRÁFICA                  #
#                                                              #
################################################################

root.mainloop()

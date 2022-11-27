'''
Python Firebase Course for
Beginners
https://codeloop.org/python-firebase-course-for-beginners/
'''
import pyrebase

#configuración de la base de datos
config = {
"apiKey": "AIzaSyDfFVXvISpNVfK6WumN7NvVeYpEEUMf8oE",
"authDomain": "hola-587b4.firebaseapp.com",
"databaseURL": "https://hola-587b4-default-rtdb.firebaseio.com",
"projectId": "hola-587b4",
"storageBucket": "hola-587b4.appspot.com",
"messagingSenderId": "393796977302",
"appId": "1:393796977302:web:38741ed0dfca9b65b678b7",
"measurementId": "G-TR04Z23HNX"
}

#conexion a la base de datos
firebase = pyrebase.initialize_app(config)
#accesando a la base de datos en firebase
db = firebase.database()
#se leen los datos de la base de datos
all_numero = db.child("app_inventor").get()
for numero in all_numero.each():
#condicion para cambiar el nombre del usuario si es mayor a 9.
  if numero.key() == "Numero" and int(numero.val()) > 9:
    db.child("app_inventor").update({"Usuario":"Elemento mayor a 9."})
#se imprimen los datos leídos con su etiqueta y el valor
    print(str(numero.key()) + ": " + str(numero.val()))
  else:
    print(str(numero.key()) + ": " + str(numero.val()))
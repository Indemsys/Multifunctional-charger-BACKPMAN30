import os
import string
import xml.etree.ElementTree as ET

prog_file_name = "Firmware.ewp"
proj_dir = os.getcwd() # Файл должен находится в корневой директории проекта


def Populate_groups(root_path, el):
  n = 0

  # Создаем группу
  grp = ET.SubElement(el, 'group')
  nm = ET.SubElement(grp, 'name')
  nm.text = os.path.basename(root_path)

  it = os.scandir(root_path)
  for entry in it:
    if not entry.name.startswith('.') and entry.is_dir():
      deeper_path = os.path.join(root_path,entry.name)
      print ('Group: ' + deeper_path)
      n = n + Populate_groups(deeper_path, grp)

  it = os.scandir(root_path)
  for entry in it:
    if not entry.name.startswith('.') and entry.is_file():
      fname = os.path.join(root_path,entry.name)
      if (fname.endswith('.c')
      or fname.endswith('.cpp')
      or fname.endswith('.s')
      or fname.endswith('.S')
      or fname.endswith('.h')
      or fname.endswith('.hpp')
      or fname.endswith('.a')):
        p = fname.replace(proj_dir,"$PROJ_DIR$")
        fl = ET.SubElement(grp, 'file')
        nm = ET.SubElement(fl,  'name')
        nm.text = p
        n = n + 1  
        print ('File:  ' + p)


  if n==0:
    el.remove(grp)
  #print (n)
  ET.indent(grp, space=" ", level=0)
  return n



full_proj_name = proj_dir + "\\" + prog_file_name

# Открываем файл проекта

tree = ET.parse(full_proj_name)
root = tree.getroot()
confs = root.findall('configuration')
for conf  in confs:
  for sett in conf.iter('settings'):
    name = sett.find('name')

    if name.text == 'ICCARM':
      set_data = sett.find('data')
      #print (name.text)
      #ET.dump(set_data)
      break

  # Преобразуем список подключаемых путей так чтобы в него попали все поддиректории корня проекта

  # Ищем тэг с перечислением подключаемых путей
  data_option_include = None
  for option in set_data.iter('option'):
    opt_name = option.find('name')
    #ET.dump(opt_name)
    if opt_name.text == 'CCIncludePath2':
      #ET.dump(option)
      data_option_include = option
      break

  if data_option_include is not None:
    #for include in data_option_include.iter('state'):
    #print (include.text)
    set_data.remove(data_option_include)

  # Восстанавливаем тэг option с перечислением путей
  new_opt = ET.SubElement(set_data, 'option')
  new_name = ET.SubElement(new_opt, 'name')
  new_name.text = 'CCIncludePath2'



  # Проходим по всем директориям проекта и включаем их в список
  # Если включать только директории содержащие .h файлы то при компиляции проекта IAR IDE не находит некоторых директорй после такого преобразования
  for dirpath, dirs, files in os.walk(proj_dir):
    if not (dirpath.startswith(proj_dir+"\\Out")
            or dirpath.startswith(proj_dir+"\\SE_proj")
            or dirpath.startswith(proj_dir+"\\Base")
            or dirpath.startswith(proj_dir+"\\.settings")
            or dirpath.startswith(proj_dir+"\\settings")
            or dirpath.startswith(proj_dir+"\\FreeMaster")
            or dirpath.startswith(proj_dir+"\\ParametersManager")
            or dirpath.startswith(proj_dir+"\\script")
            or dirpath.startswith(proj_dir+"\\OUTPUT")
            or dirpath.startswith(proj_dir+"\\Out")
            or dirpath.startswith(proj_dir+"\\debug")
            or dirpath.startswith(proj_dir+"\\.hg")
            or dirpath.startswith(proj_dir+"\\Docs")
            or (dirpath == proj_dir)):
      p = dirpath.replace(proj_dir, "$PROJ_DIR$")
      new_incl = ET.SubElement(new_opt, 'state')
      new_incl.text = p
      print('Include: ' + p)

  ET.indent(new_opt, space=" ", level=0)

  
# Удаляем список файлов и формируем новый с распределением по группам аналогичным распределению по директориям



groups = root.findall('group')
for group  in groups:
  #ET.dump(group)
  root.remove(group)
  
  
Populate_groups(proj_dir , root)  
  
    

print ("END!")
tree.write(full_proj_name, method="xml", xml_declaration=True, short_empty_elements=False, encoding="UTF-8")



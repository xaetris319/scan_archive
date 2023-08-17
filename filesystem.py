import os.path

import messages


def make_images_list(work_dir: str, input_segments: list) -> (list, str, str):
    images = list()
    msgtype = messages.TYPE_OK
    msgtext = ''

    non_existing = list()

    for i in range(len(input_segments)):
        if i == 0:  # первый фрагмент задает режим работы программы, пропускаем
            continue

        image = f'{work_dir}{input_segments[i]}'
        if os.path.exists(image):
            images.append(image)  # если файл существует, он добавится в выводимый список
        else:
            non_existing.append(input_segments[i])

    if len(non_existing) > 0:  # если были найдены несуществующие файлы, они фиксируются в выводе
        msgtype = messages.TYPE_WARNING
        for elem in non_existing:
            if msgtext == '':
                msgtext = f'{elem}'
            else:
                msgtext = f'{msgtext}; {elem}'

        msgtext = f'{messages.CANNOT_OPEN_IMAGE}: {msgtext}'

    if len(images) == 0:
        msgtype = messages.TYPE_ERROR
        msgtext = messages.WRONG_INPUT

    return images, msgtype, msgtext

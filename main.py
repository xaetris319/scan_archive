import os

import messages
import iooperations
import naps2
import filesystem
import zbar
from shutil import rmtree


def entry():
    work_dir = os.path.expandvars(b'%APPDATA%\\SAP\\ScanArchive\\').decode('utf8')
    input_segments, msgtype, msgtext = iooperations.read_input(work_dir)

    if msgtype == messages.TYPE_ERROR:
        iooperations.write_output(work_dir, msgtype, msgtext)
        return

    images = list()

    match input_segments[0]:  # режим работы программы
        case '1':
            # Сканирование.
            images, msgtype, msgtext = naps2.perform_scanning(work_dir, input_segments)

        case '2':
            # Считывание из папки.
            images, msgtype, msgtext = filesystem.make_images_list(work_dir, input_segments)

        case '_':
            msgtype = messages.TYPE_ERROR
            msgtext = messages.WRONG_INPUT

    if msgtype == messages.TYPE_ERROR:
        iooperations.write_output(work_dir, msgtype, msgtext)
        return

    output_content = ''

    filtered_dir = f'{work_dir}\\filtered_temp'

    # Вообще, на данном этапе гарантируется, что список изображений не пустой.
    if len(images) > 0 and not os.path.exists(filtered_dir):
        os.makedirs(filtered_dir)

    for image in images:
        all_barcode_content = zbar.perform_decoding(image, filtered_dir)
        if len(all_barcode_content) == 0:
            output_content = f'{output_content}{image};'
        else:
            for barcode_content in all_barcode_content:
                output_content = f'{output_content}{image},{barcode_content["type"]},{barcode_content["data"]};'

    rmtree(filtered_dir)
    iooperations.write_output(work_dir, msgtype, msgtext, output_content)
    return


if __name__ == '__main__':
    entry()

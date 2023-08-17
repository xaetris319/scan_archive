import os.path
from datetime import datetime
import subprocess

import messages


def perform_scanning(work_dir: str, input_segments: list) -> (list, str, str):
    # Перед вызовом уже была произведена проверка, что в списке минимум 3 элемента.
    try:
        profile_name = input_segments[1]
        file_extension = input_segments[2]
    except IndexError:
        print(messages.WRONG_INPUT)
        return list(), messages.TYPE_ERROR, messages.WRONG_INPUT

    if profile_name == '' or file_extension == '':
        print(messages.WRONG_INPUT)
        return list(), messages.TYPE_ERROR, messages.WRONG_INPUT

    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    saveto_dir = f'{work_dir}{timestamp}\\'

    console_path = '"C:\\Program Files (x86)\\NAPS2\\naps2.console"'
    profile_option = f'-p {profile_name}'
    output_option = f'-o {saveto_dir}DOC$(nnn).{file_extension}'
    other_options = '--FORCE --SPLIT'

    naps2_command = f'{console_path} {profile_option} {output_option} {other_options}'

    process = subprocess.run(naps2_command, capture_output=True, shell=True)
    output_text = process.stdout.decode('cp866').replace('\r', '').replace('\n', ' ')

    if output_text == '' and os.path.isdir(saveto_dir):
        image_short_names = os.listdir(saveto_dir)
        images = list(map(lambda x: f'{saveto_dir}{x}', image_short_names))
        if len(images) == 0:
            msgtype = messages.TYPE_ERROR
            msgtext = messages.NO_SCANS
        else:
            msgtype = messages.TYPE_OK
            msgtext = ''
    else:
        print(output_text)
        images = None
        msgtype = messages.TYPE_ERROR
        msgtext = output_text

    return images, msgtype, msgtext

import io
import chardet

import messages


def read_input(work_dir: str) -> (list, str, str):
    input_segments = None

    try:
        with io.open(f'{work_dir}\\input.txt', 'rb') as input_file:
            input_content_raw = input_file.read()
            # Предполагается, что файл будет в UTF-8, иначе могут быть различные ошибки.
            detector = chardet.detect(input_content_raw)
            input_content = input_content_raw.decode(detector['encoding']).replace('\r', '').replace('\n', '')
            input_segments = input_content.split(';')
            try:
                if len(input_segments) < 2 or input_segments[1] == '':
                    msgtype = messages.TYPE_ERROR
                    msgtext = messages.WRONG_INPUT
                else:
                    msgtype = messages.TYPE_OK
                    msgtext = ''
            except IndexError:
                msgtype = messages.TYPE_ERROR
                msgtext = messages.WRONG_INPUT

    except OSError:
        print(messages.CANNOT_OPEN_INPUT)
        msgtype = messages.TYPE_ERROR
        msgtext = messages.CANNOT_OPEN_INPUT

    return input_segments, msgtype, msgtext


def write_output(work_dir: str, msgtype: str, msgtext: str, output_content=''):
    try:
        with io.open(f'{work_dir}\\output.txt', 'w', encoding='utf8') as f:
            f.write(f'{msgtype};{msgtext};{output_content}')

    except OSError:
        print(messages.CANNOT_OPEN_OUTPUT)

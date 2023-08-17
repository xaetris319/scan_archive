from PIL import Image
from pyzbar.pyzbar import decode, ZBarSymbol
import re
import subprocess

REC601 = '-enhance -grayscale Rec601Luminance'
REC709 = '-enhance -grayscale Rec709Luminance'
REC601_WT50 = '-enhance -grayscale Rec601Luminance -white-threshold 50%'
REC709_WT50 = '-enhance -grayscale Rec709Luminance -white-threshold 50%'
REC601_WT70 = '-enhance -grayscale Rec601Luminance -white-threshold 70%'
REC709_WT70 = '-enhance -grayscale Rec709Luminance -white-threshold 70%'
REC601_WT90 = '-enhance -grayscale Rec601Luminance -white-threshold 90%'
REC709_WT90 = '-enhance -grayscale Rec709Luminance -white-threshold 90%'
REC601_AUTOTS = '-enhance -grayscale Rec601Luminance -auto-threshold OTSU'
REC709_AUTOTS = '-enhance -grayscale Rec709Luminance -auto-threshold OTSU'
REC601_BTWT30 = '-enhance -grayscale Rec601Luminance -black-threshold 30% -white-threshold 30%'
REC601_BTWT50 = '-enhance -grayscale Rec601Luminance -black-threshold 50% -white-threshold 50%'
REC601_BTWT70 = '-enhance -grayscale Rec601Luminance -black-threshold 70% -white-threshold 70%'

PAGE_NUMBER_REGEXP = 'P\\d+?\\$\\d+?'
MAIN_BARCODE_REGEXP = '\\d{3}\\.\\|S\\|K\\d+?\\.\\d{8}|\\d{3}\\.\\d+?\\.\\d{8}|\\d{3}\\.\\d+?-\\d+?-\\d{8}'


def get_short_name(full_name: str):
    name_segments = full_name.split('\\')
    return name_segments[len(name_segments) - 1]


def create_filtered_image(image: str, filtered_dir: str, filter_option):
    filtered_image = f'{filtered_dir}\\{filter_option} {get_short_name(image)}'
    magick_command = f'magick \"{image}\" {filter_option} \"{filtered_image}\"'
    process = subprocess.run(magick_command, capture_output=True, shell=True)

    if process.stdout != b'':  # если Magick отработал с ошибкой, возвращать путь не будем
        filtered_image = None

    return filtered_image


def perform_decoding(image: str, filtered_dir: str):
    page_number = main_barcode = ''
    all_barcode_content = list()
    zbar_symbols = [ZBarSymbol.CODE39, ZBarSymbol.CODE128]
    filter_options = [REC601, REC709, REC601_WT70, REC709_WT70]

    image_object = Image.open(image)

    if image_object is not None:
        barcodes = decode(image_object, zbar_symbols)
        for barcode in barcodes:
            data = barcode.data.decode('utf8')
            barcode_content = {"type": barcode.type,
                               "data": data}
            if barcode_content not in all_barcode_content:
                all_barcode_content.append(barcode_content)
                if re.search(PAGE_NUMBER_REGEXP, data) is not None:
                    page_number = data
                if re.search(MAIN_BARCODE_REGEXP, data) is not None:
                    main_barcode = data

        if page_number == '' or main_barcode == '':
            for filter_option in filter_options:
                filtered_image = create_filtered_image(image, filtered_dir, filter_option)
                if filtered_image is None:
                    continue

                try:
                    filtered_image_obj = Image.open(filtered_image)
                except OSError:
                    continue
                if filtered_image_obj is None:
                    continue

                barcodes = decode(filtered_image_obj, zbar_symbols)
                for barcode in barcodes:
                    data = barcode.data.decode('utf8')
                    barcode_content = {"type": barcode.type,
                                       "data": data}
                    if barcode_content not in all_barcode_content:
                        all_barcode_content.append(barcode_content)
                        if page_number == '' and re.search(PAGE_NUMBER_REGEXP, data) is not None:
                            page_number = data
                        if main_barcode == '' and re.search(MAIN_BARCODE_REGEXP, data) is not None:
                            main_barcode = data

                if page_number != '' and main_barcode != '':
                    break

    return all_barcode_content

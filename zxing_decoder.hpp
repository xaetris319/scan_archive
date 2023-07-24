

class zxing_decoder
{
public:
    static void decode_image(const std::string &work_dir,
                             const std::string &image,
                             std::string &barcodes_entry)
    {
        std::string magick_command = "C:\\Program Files (x86)\\ImageMagick\\magick.exe " +
                                     image + " -grayscale Rec709Luminance " + image + "_gray1";
        os_operations::exec(magick_command.c_str());
    }

    static std::vector<std::string> decode_images(const std::string &saveto_dir,
                                                  const std::vector<std::string> &images_list)
    {
        size_t start_pos = saveto_dir.length();
        for (std::string const &image : images_list)
        {
            std::vector<std::string> image_copies;
            std::string short_name = image.substr(start_pos, image.length() - start_pos);
            image_copies.push_back(saveto_dir + "gray1_" + short_name);
            image_copies.push_back(saveto_dir + "gray2_" + short_name);
            image_copies.push_back(saveto_dir + "wt_" + short_name);

            std::string magick_command = "C:\\Program Files (x86)\\ImageMagick\\magick.exe " +
                                         image + " -grayscale Rec709Luminance " +
                                         saveto_dir + "gray1_" + short_name;
        }
    };

    // std::ostream &operator<<(std::ostream &os, const ZXing::Position &points)
    // {
    //     for (const auto &p : points)
    //         os << p.x << "x" << p.y << " ";
    //     return os;
    // }

    static void drawLine(const ZXing::ImageView &iv, ZXing::PointI a, ZXing::PointI b, bool error)
    {
        int steps = maxAbsComponent(b - a);
        ZXing::PointF dir = ZXing::bresenhamDirection(ZXing::PointF(b - a));
        int R = RedIndex(iv.format()), G = GreenIndex(iv.format()), B = BlueIndex(iv.format());
        for (int i = 0; i < steps; ++i)
        {
            auto p = ZXing::PointI(centered(a + i * dir));
            auto *dst = const_cast<uint8_t *>(iv.data(p.x, p.y));
            if (dst < iv.data(0, 0) || dst > iv.data(iv.width() - 1, iv.height() - 1))
                continue;
            dst[R] = error ? 0xff : 0;
            dst[G] = error ? 0 : 0xff;
            dst[B] = 0;
        }
    }

    static void drawRect(const ZXing::ImageView &image, const ZXing::Position &pos, bool error)
    {
        for (int i = 0; i < 4; ++i)
            drawLine(image, pos[i], pos[(i + 1) % 4], error);
    }

    static int test()
    {
        ZXing::DecodeHints hints;
        std::vector<std::string> filePaths;
        ZXing::Results allResults;
        std::string outPath;
        bool oneLine = false;
        bool bytesOnly = false;
        int ret = 0;

        hints.setTextMode(ZXing::TextMode::HRI);
        hints.setEanAddOnSymbol(ZXing::EanAddOnSymbol::Read);

        // if (!ParseOptions(argc, argv, hints, oneLine, bytesOnly, filePaths, outPath))
        // {
        //     PrintUsage(argv[0]);
        //     return -1;
        // }

        std::cout.setf(std::ios::boolalpha);

        for (const auto &filePath : filePaths)
        {
            int width, height, channels;
            std::unique_ptr<stbi_uc, void (*)(void *)> buffer(stbi_load(filePath.c_str(), &width, &height, &channels, 3), stbi_image_free);
            if (buffer == nullptr)
            {
                std::cerr << "Failed to read image: " << filePath << "\n";
                return -1;
            }

            ZXing::ImageView image{buffer.get(), width, height, ZXing::ImageFormat::RGB};
            auto results = ReadBarcodes(image, hints);

            // if we did not find anything, insert a dummy to produce some output for each file
            if (results.empty())
                results.emplace_back();

            allResults.insert(allResults.end(), results.begin(), results.end());
            if (filePath == filePaths.back())
            {
                auto merged = MergeStructuredAppendSequences(allResults);
                // report all merged sequences as part of the last file to make the logic not overly complicated here
                results.insert(results.end(), std::make_move_iterator(merged.begin()), std::make_move_iterator(merged.end()));
            }

            for (auto &&result : results)
            {

                if (!outPath.empty())
                    drawRect(image, result.position(), bool(result.error()));

                ret |= static_cast<int>(result.error().type());

                if (bytesOnly)
                {
                    std::cout.write(reinterpret_cast<const char *>(result.bytes().data()), result.bytes().size());
                    continue;
                }

                if (oneLine)
                {
                    std::cout << filePath << " " << ToString(result.format());
                    if (result.isValid())
                        std::cout << " \"" << result.text(ZXing::TextMode::Escaped) << "\"";
                    else if (result.error())
                        std::cout << " " << ToString(result.error());
                    std::cout << "\n";
                    continue;
                }

                if (filePaths.size() > 1 || results.size() > 1)
                {
                    static bool firstFile = true;
                    if (!firstFile)
                        std::cout << "\n";
                    if (filePaths.size() > 1)
                        std::cout << "File:       " << filePath << "\n";
                    firstFile = false;
                }

                if (result.format() == ZXing::BarcodeFormat::None)
                {
                    std::cout << "No barcode found\n";
                    continue;
                }

                std::cout << "Text:       \"" << result.text() << "\"\n"
                          << "Bytes:      " << ZXing::ToHex(hints.textMode() == ZXing::TextMode::ECI ? result.bytesECI() : result.bytes()) << "\n"
                          << "Format:     " << ToString(result.format()) << "\n"
                          << "Identifier: " << result.symbologyIdentifier() << "\n"
                          << "Content:    " << ToString(result.contentType()) << "\n"
                          << "HasECI:     " << result.hasECI() << "\n"
                          //   << "Position:   " << result.position() << "\n"
                          << "Rotation:   " << result.orientation() << " deg\n"
                          << "IsMirrored: " << result.isMirrored() << "\n"
                          << "IsInverted: " << result.isInverted() << "\n";

                auto printOptional = [](const char *key, const std::string &v)
                {
                    if (!v.empty())
                        std::cout << key << v << "\n";
                };

                printOptional("EC Level:   ", result.ecLevel());
                printOptional("Version:    ", result.version());
                printOptional("Error:      ", ToString(result.error()));

                if (result.lineCount())
                    std::cout << "Lines:      " << result.lineCount() << "\n";

                if ((ZXing::BarcodeFormat::EAN13 | ZXing::BarcodeFormat::EAN8 | ZXing::BarcodeFormat::UPCA | ZXing::BarcodeFormat::UPCE)
                        .testFlag(result.format()))
                {
                    printOptional("Country:    ", ZXing::GTIN::LookupCountryIdentifier(result.text(), result.format()));
                    printOptional("Add-On:     ", ZXing::GTIN::EanAddOn(result));
                    printOptional("Price:      ", ZXing::GTIN::Price(ZXing::GTIN::EanAddOn(result)));
                    printOptional("Issue #:    ", ZXing::GTIN::IssueNr(ZXing::GTIN::EanAddOn(result)));
                }
                else if (result.format() == ZXing::BarcodeFormat::ITF && Size(result.bytes()) == 14)
                {
                    printOptional("Country:    ", ZXing::GTIN::LookupCountryIdentifier(result.text(), result.format()));
                }

                if (result.isPartOfSequence())
                    std::cout << "Structured Append: symbol " << result.sequenceIndex() + 1 << " of "
                              << result.sequenceSize() << " (parity/id: '" << result.sequenceId() << "')\n";
                else if (result.sequenceSize() > 0)
                    std::cout << "Structured Append: merged result from " << result.sequenceSize() << " symbols (parity/id: '"
                              << result.sequenceId() << "')\n";

                if (result.readerInit())
                    std::cout << "Reader Initialisation/Programming\n";
            }

            if (ZXing::Size(filePaths) == 1 && !outPath.empty())
                stbi_write_png(outPath.c_str(), image.width(), image.height(), 3, image.data(0, 0), image.rowStride());

#ifdef NDEBUG
            if (getenv("MEASURE_PERF"))
            {
                auto startTime = std::chrono::high_resolution_clock::now();
                auto duration = startTime - startTime;
                int N = 0;
                int blockSize = 1;
                do
                {
                    for (int i = 0; i < blockSize; ++i)
                        ReadBarcodes(image, hints);
                    N += blockSize;
                    duration = std::chrono::high_resolution_clock::now() - startTime;
                    if (blockSize < 1000 && duration < std::chrono::milliseconds(100))
                        blockSize *= 10;
                } while (duration < std::chrono::seconds(1));
                printf("time: %5.2f ms per frame\n", double(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()) / N);
            }
#endif
        }

        return ret;
    }

private:
    zxing_decoder() {}
    ~zxing_decoder() {}
};
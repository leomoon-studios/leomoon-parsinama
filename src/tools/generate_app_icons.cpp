#include <QBuffer>
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

#include <cstdio>

namespace {

QImage renderIcon(QSvgRenderer &renderer, int size)
{
    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter);
    return image;
}

bool writeAndroidIcons(QSvgRenderer &renderer, const QString &directory)
{
    const struct { const char *density; int size; } variants[] = {
        {"mdpi", 48}, {"hdpi", 72}, {"xhdpi", 96},
        {"xxhdpi", 144}, {"xxxhdpi", 192}
    };
    for (const auto &variant : variants) {
        const QString path = directory + QStringLiteral("/mipmap-")
            + QLatin1String(variant.density) + QStringLiteral("/ic_launcher.png");
        if (!QDir().mkpath(QFileInfo(path).absolutePath())
            || !renderIcon(renderer, variant.size).save(path, "PNG")) {
            std::fprintf(stderr, "Could not write Android icon: %s\n", qPrintable(path));
            return false;
        }
    }
    return true;
}

bool writeWindowsIcon(QSvgRenderer &renderer, const QString &path)
{
    const int sizes[] = {16, 24, 32, 48, 64, 128, 256};
    QList<QByteArray> images;
    for (int size : sizes) {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        if (!buffer.open(QIODevice::WriteOnly)
            || !renderIcon(renderer, size).save(&buffer, "PNG")) {
            std::fprintf(stderr, "Could not render Windows icon size %d\n", size);
            return false;
        }
        images.append(bytes);
    }

    if (!QDir().mkpath(QFileInfo(path).absolutePath()))
        return false;
    QFile output(path);
    if (!output.open(QIODevice::WriteOnly)) {
        std::fprintf(stderr, "Could not write Windows icon: %s\n", qPrintable(path));
        return false;
    }
    QDataStream stream(&output);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint16(0) << quint16(1) << quint16(images.size());
    quint32 offset = 6 + images.size() * 16;
    for (int index = 0; index < images.size(); ++index) {
        stream << quint8(sizes[index] == 256 ? 0 : sizes[index]);
        stream << quint8(sizes[index] == 256 ? 0 : sizes[index]);
        stream << quint8(0) << quint8(0) << quint16(1) << quint16(32);
        stream << quint32(images[index].size()) << offset;
        offset += images[index].size();
    }
    for (const QByteArray &image : images)
        stream.writeRawData(image.constData(), image.size());
    return stream.status() == QDataStream::Ok;
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    const QStringList arguments = application.arguments();
    if (arguments.size() != 4 || (arguments[1] != QLatin1String("android")
                                  && arguments[1] != QLatin1String("windows"))) {
        std::fprintf(stderr, "Usage: parsinama-icon-generator android|windows source.svg output\n");
        return 1;
    }
    QSvgRenderer renderer(arguments[2]);
    if (!renderer.isValid()) {
        std::fprintf(stderr, "Could not read SVG icon: %s\n", qPrintable(arguments[2]));
        return 1;
    }
    const bool success = arguments[1] == QLatin1String("android")
        ? writeAndroidIcons(renderer, arguments[3])
        : writeWindowsIcon(renderer, arguments[3]);
    return success ? 0 : 1;
}

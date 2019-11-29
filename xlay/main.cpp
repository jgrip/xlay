#include "lay6.h"
#include <stdio.h>
#include <xmllite.h>

#pragma comment(lib, "xmllite.lib")

class StdOut : public ISequentialStream
{
private:
    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override
    {
        if (IsEqualIID(riid, IID_ISequentialStream))
        {
            *ppvObject = (ISequentialStream *) this;
            return S_OK;
        }
        else if (IsEqualIID(riid, IID_IUnknown))
        {
            *ppvObject = (IUnknown *) this;
            return S_OK;
        }
        else
        {
            return E_NOINTERFACE;
        }
    }

    ULONG STDMETHODCALLTYPE AddRef(void) override
    {
        return 1;
    }

    ULONG STDMETHODCALLTYPE Release(void) override
    {
        return 1;
    }

    // ISequentialStream
    HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead) override
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb, ULONG *pcbWritten) override
    {
        const char *pc = (const char *) pv;
        *pcbWritten = cb;
        while (cb > 0)
        {
            putc(*pc++, stdout);
            cb--;
        }
        return S_OK;
    }
};

static void xmlAttr(IXmlWriter *pw, LPCWSTR name, float a)
{
    CStringW val;
    val.Format(L"%f", a);
    pw->WriteAttributeString(NULL, name, NULL, val);
}

static void xmlAttrU(IXmlWriter *pw, LPCWSTR name, UINT a)
{
    CStringW val;
    val.Format(L"%u", a);
    pw->WriteAttributeString(NULL, name, NULL, val);
}

static void xmlAttrX(IXmlWriter *pw, LPCWSTR name, UCHAR *v, int len)
{
    CStringW val;
    for (int i = 0; i < len; i++)
    {
        val.AppendFormat(L"%02x", v[i]);
    }
    pw->WriteAttributeString(NULL, name, NULL, val);
}

int main()
{
    IXmlWriter *pXmlWriter;
    StdOut stdOut;
    FILE *lay;
    HRESULT hr;

    errno = fopen_s(&lay, "C:\\Users\\Sergey.Raevskiy\\Downloads\\test9.lay6", "rb");
    if (errno)
    {
        printf("fopen_s() failed: %d\n", errno);
        return EXIT_FAILURE;
    }

    hr = CreateXmlWriter(IID_IXmlWriter, (void **) &pXmlWriter, NULL);
    if (FAILED(hr))
    {
        printf("CreateXmlWriter() failed: %x\n", hr);
        return EXIT_FAILURE;
    }

    pXmlWriter->SetOutput(&stdOut);
    pXmlWriter->SetProperty(XmlWriterProperty_Indent, TRUE);

    pXmlWriter->WriteStartElement(NULL, L"xlay", NULL);

    CLayFileHeader fileHeader;
    fileHeader.Read(lay);

    for (int i = 0; i < fileHeader.num_boards; i++)
    {
        CLayBoardHeader bhdr;
        bhdr.Read(lay);

        CStringW wname(bhdr.name.str());
        CStringW sizeX;
        CStringW sizeY;
        CStringW zoom;
        CStringW centerX;
        CStringW centerY;
        CStringW numObjects;
        //CStringW unk(bhdr.unk22.str());

        sizeX.Format(L"%d", bhdr.size_x);
        sizeY.Format(L"%d", bhdr.size_y);
        zoom.Format(L"%d", UINT(bhdr.zoom));
        centerX.Format(L"%d", bhdr.center_x);
        centerY.Format(L"%d", bhdr.center_y);
        numObjects.Format(L"%u", bhdr.num_objects);

        pXmlWriter->WriteStartElement(NULL, L"bhdr", NULL);
        pXmlWriter->WriteAttributeString(NULL, L"name", NULL, wname);
        pXmlWriter->WriteAttributeString(NULL, L"sizeX", NULL, sizeX);
        pXmlWriter->WriteAttributeString(NULL, L"sizeY", NULL, sizeY);
        pXmlWriter->WriteAttributeString(NULL, L"activeGridStep", NULL, CStringW(bhdr.active_grid_val.str()));
        pXmlWriter->WriteAttributeString(NULL, L"zoom", NULL, zoom);
        pXmlWriter->WriteAttributeString(NULL, L"centerX", NULL, centerX);
        pXmlWriter->WriteAttributeString(NULL, L"centerY", NULL, centerY);
        pXmlWriter->WriteAttributeString(NULL, L"numObjects", NULL, numObjects);
        pXmlWriter->WriteEndElement();
    }

    CLayObject obj;
    obj.Read(lay);

    pXmlWriter->WriteStartElement(NULL, L"obj", NULL);
    xmlAttr(pXmlWriter, L"x", obj.x);
    xmlAttr(pXmlWriter, L"y", obj.y);
    xmlAttr(pXmlWriter, L"out", obj.out); // radius for tht, height for text
    xmlAttr(pXmlWriter, L"in", obj.in); // also line width for text
    xmlAttrU(pXmlWriter, L"metal", obj.metalisation);
    xmlAttrU(pXmlWriter, L"thermo", obj.thermobarier); // flip horizontal for text
    xmlAttrU(pXmlWriter, L"th_style_custom", obj.th_style_custom);
    xmlAttrU(pXmlWriter, L"shape", obj.tht_shape);
    xmlAttrU(pXmlWriter, L"thzise", obj.thzise);
    xmlAttrX(pXmlWriter, L"th_style", obj.th_style, sizeof(obj.th_style));
    xmlAttrU(pXmlWriter, L"layer", obj.layer);
    xmlAttrU(pXmlWriter, L"cutoff", obj.cutoff);
    xmlAttrU(pXmlWriter, L"line-width", obj.line_width); // also text style?
    xmlAttrU(pXmlWriter, L"flip-vertical", obj.flip_vertical); // also text style?
    pXmlWriter->WriteAttributeString(NULL, L"text", NULL, CStringW(obj.text, obj.text_len));
    pXmlWriter->WriteEndElement();

    pXmlWriter->WriteEndElement();

    pXmlWriter->Flush();

    printf("%d\n", sizeof(LAY_BoardHeader));
    printf("%x\n", FIELD_OFFSET(LAY_BoardHeader, size_x));
    printf("%x\n", FIELD_OFFSET(LAY_BoardHeader, layer_visible));
    system("pause");
}

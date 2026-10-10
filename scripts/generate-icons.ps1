# Original VIMEK vector geometry -> transparent PNG, ICO and ICNS. No external tools.
param([switch]$TrayOnly)
$ErrorActionPreference = 'Stop'
$vimekRoot = Split-Path -Parent $PSScriptRoot
$vimekBrand = Join-Path $vimekRoot 'assets/brand'
New-Item -ItemType Directory -Force $vimekBrand | Out-Null
Add-Type -AssemblyName System.Drawing
# Load drawing dependencies before compiling, including the split Windows
# drawing assemblies in newer PowerShell/.NET versions.
$vimekDrawingProbe = [System.Drawing.Bitmap]::new(1,1)
$vimekDrawingProbe.Dispose()
$vimekDrawingAssemblies = [AppDomain]::CurrentDomain.GetAssemblies() | Where-Object { $_.GetName().Name -match '^(System.Drawing|System.Private.Windows)' } | Select-Object -ExpandProperty Location -Unique
Add-Type -ReferencedAssemblies $vimekDrawingAssemblies -TypeDefinition @'
using System;
using System.IO;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
public static class VimekIcons {
    static PointF[] Shape(bool english) {
        float[] v={14,13,23,13,32,40,41,13,50,13,36,51,28,51};
        float[] e={17,13,48,13,48,20,25,20,25,29,45,29,45,36,25,36,25,44,48,44,48,51,17,51};
        float[] a=english?e:v;var p=new PointF[a.Length/2];
        for(int i=0;i<p.Length;i++)p[i]=new PointF(a[i*2],a[i*2+1]);return p;
    }
    public static string Svg(bool english) {
        string points="";foreach(var p in Shape(english))points+=p.X+","+p.Y+" ";
        return "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 64 64\"><g fill=\"none\" stroke-linejoin=\"round\"><polygon points=\""+points+"\" stroke=\"#171b22\" stroke-opacity=\".72\" stroke-width=\"2.8\"/><polygon points=\""+points+"\" stroke=\"white\" stroke-width=\"1.65\"/></g></svg>";
    }
    public static byte[] Png(bool english,int size,bool tray=false) {
        int ss=size<256?8:2;
        using(var canvas=new Bitmap(size*ss,size*ss,PixelFormat.Format32bppArgb)) {
            using(var g=Graphics.FromImage(canvas)) {
                g.Clear(Color.Transparent);g.SmoothingMode=SmoothingMode.AntiAlias;
                using(var path=new GraphicsPath()) {
                    var points=Shape(english);
                    if(tray) {
                        // Use the notification area's full height. App/brand
                        // assets retain their padding; tray letters need to
                        // remain legible in a 16px slot at 100% display scale.
                        float centerX=english?32.5f:32f;
                        for(int i=0;i<points.Length;i++)points[i]=new PointF(
                            32+(points[i].X-centerX)*54f/38f,
                            32+(points[i].Y-32)*54f/38f);
                    }
                    path.AddPolygon(points);using(var m=new Matrix()){m.Scale(size*ss/64f,size*ss/64f);path.Transform(m);}
                    float width=Math.Max(size/64f*1.65f,0.95f)*ss;
                    using(var shadow=new Pen(Color.FromArgb(185,23,27,34),width+Math.Max(size/64f*1.15f,0.65f)*ss)){
                        shadow.LineJoin=LineJoin.Round;g.DrawPath(shadow,path);
                    }
                    using(var white=new Pen(Color.White,width)){white.LineJoin=LineJoin.Round;g.DrawPath(white,path);}
                }
            }
            using(var final=new Bitmap(size,size,PixelFormat.Format32bppArgb)) {
                using(var g=Graphics.FromImage(final)){g.CompositingMode=CompositingMode.SourceCopy;g.InterpolationMode=InterpolationMode.HighQualityBicubic;g.DrawImage(canvas,new Rectangle(0,0,size,size),0,0,canvas.Width,canvas.Height,GraphicsUnit.Pixel);}
                using(var stream=new MemoryStream()){final.Save(stream,ImageFormat.Png);return stream.ToArray();}
            }
        }
    }
    public static void Ico(string file,bool english,bool tray=false) {
        int[] sizes={16,20,24,32,40,48,64,128,256};var pngs=new byte[sizes.Length][];
        for(int i=0;i<sizes.Length;i++)pngs[i]=Png(english,sizes[i],tray);
        using(var w=new BinaryWriter(File.Create(file))) {
            w.Write((ushort)0);w.Write((ushort)1);w.Write((ushort)sizes.Length);int offset=6+16*sizes.Length;
            for(int i=0;i<sizes.Length;i++){w.Write((byte)(sizes[i]==256?0:sizes[i]));w.Write((byte)(sizes[i]==256?0:sizes[i]));w.Write((ushort)0);w.Write((ushort)1);w.Write((ushort)32);w.Write(pngs[i].Length);w.Write(offset);offset+=pngs[i].Length;}
            foreach(var png in pngs)w.Write(png);
        }
    }
    static void Big(BinaryWriter w,int n){w.Write(new byte[]{(byte)(n>>24),(byte)(n>>16),(byte)(n>>8),(byte)n});}
    public static void Icns(string file) {
        int[] sizes={16,32,64,128,256,512,1024};string[] tags={"icp4","icp5","icp6","ic07","ic08","ic09","ic10"};
        var pngs=new byte[sizes.Length][];int total=8;
        for(int i=0;i<sizes.Length;i++){pngs[i]=Png(false,sizes[i]);total+=8+pngs[i].Length;}
        using(var w=new BinaryWriter(File.Create(file))){w.Write(System.Text.Encoding.ASCII.GetBytes("icns"));Big(w,total);
            for(int i=0;i<sizes.Length;i++){w.Write(System.Text.Encoding.ASCII.GetBytes(tags[i]));Big(w,8+pngs[i].Length);w.Write(pngs[i]);}}
    }
}
'@
$vimekWin = Join-Path $vimekRoot 'Sources/VIMEK/windows/App'
$vimekMac = Join-Path $vimekRoot 'Sources/VIMEK/macOS/App/Resources'
foreach ($vimekEnglish in @($false,$true)) {
    $vimekLetter = if ($vimekEnglish) { 'e' } else { 'v' }
    $vimekStatus = if ($vimekEnglish) { 'StatusEng' } else { 'StatusViet' }
    [VimekIcons]::Ico((Join-Path $vimekWin "$vimekStatus.ico"),$vimekEnglish,$true)
    [VimekIcons]::Ico((Join-Path $vimekWin "$($vimekStatus)10.ico"),$vimekEnglish,$true)
    if ($TrayOnly) { continue }
    [IO.File]::WriteAllText((Join-Path $vimekBrand "$vimekLetter.svg"),[VimekIcons]::Svg($vimekEnglish))
    [IO.File]::WriteAllBytes((Join-Path $vimekBrand "$vimekLetter.png"),[VimekIcons]::Png($vimekEnglish,256))
    $vimekMacStatus = if ($vimekEnglish) { 'StatusEng' } else { 'Status' }
    $vimekMacHighlight = if ($vimekEnglish) { 'StatusHighlightedEng' } else { 'StatusHighlighted' }
    foreach ($vimekName in @($vimekMacStatus,$vimekMacHighlight)) {
        [IO.File]::WriteAllBytes((Join-Path $vimekMac "$vimekName.png"),[VimekIcons]::Png($vimekEnglish,18))
        [IO.File]::WriteAllBytes((Join-Path $vimekMac "$vimekName@2x.png"),[VimekIcons]::Png($vimekEnglish,36))
    }
}
if (-not $TrayOnly) {
    [VimekIcons]::Ico((Join-Path $vimekWin 'icon.ico'),$false)
    [VimekIcons]::Icns((Join-Path $vimekMac 'Icon.icns'))
}
Write-Host 'Generated larger Windows tray V/E icons (transparent outline, 9 ICO sizes).'

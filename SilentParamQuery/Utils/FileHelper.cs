using System.IO;
using System.Text;

namespace SilentParamQuery.Utils
{
    public static class FileHelper
    {
        public static bool IsExeFile(string filePath)
        {
            if (string.IsNullOrEmpty(filePath) || !File.Exists(filePath))
                return false;

            string ext = Path.GetExtension(filePath).ToLower();
            return ext == ".exe" || ext == ".msi" || ext == ".appx" || ext == ".msix" || ext == ".msixbundle";
        }

        public static string GetFileSize(string filePath)
        {
            if (!File.Exists(filePath)) return "未知";
            long size = new FileInfo(filePath).Length;
            if (size < 1024) return $"{size} B";
            if (size < 1024 * 1024) return $"{size / 1024.0:F1} KB";
            return $"{size / (1024.0 * 1024.0):F1} MB";
        }

        public static void CopyToClipboard(string text)
        {
            try
            {
                System.Windows.Forms.Clipboard.SetText(text);
            }
            catch { }
        }
    }
}

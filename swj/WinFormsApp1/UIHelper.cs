using System.Drawing.Drawing2D;
using System.Windows.Forms;

namespace WinFormsApp1
{
    /// <summary>
    /// UI 美化助手类
    /// </summary>
    public static class UIHelper
    {
        // 定义统一的颜色方案
        public static class Colors
        {
            public static Color Primary = Color.FromArgb(52, 152, 219);       // 蓝色
            public static Color Secondary = Color.FromArgb(46, 204, 113);     // 绿色
            public static Color Danger = Color.FromArgb(231, 76, 60);         // 红色
            public static Color Warning = Color.FromArgb(241, 196, 15);       // 黄色
            public static Color Info = Color.FromArgb(149, 165, 166);         // 灰色
            public static Color Background = Color.FromArgb(236, 240, 241);   // 浅灰背景
            public static Color DarkText = Color.FromArgb(44, 62, 80);        // 深灰文字
            public static Color LightText = Color.FromArgb(255, 255, 255);    // 白色文字
            public static Color BorderColor = Color.FromArgb(189, 195, 199);  // 边框颜色
        }

        /// <summary>
        /// 设置圆角按钮样式
        /// </summary>
        public static void StyleRoundButton(Button btn, Color backColor, Color foreColor, int cornerRadius = 10)
        {
            btn.FlatStyle = FlatStyle.Flat;
            btn.FlatAppearance.BorderSize = 0;
            btn.BackColor = backColor;
            btn.ForeColor = foreColor;
            btn.Font = new Font("微软雅黑", 11, FontStyle.Bold);
            btn.Height = 40;
            btn.Cursor = Cursors.Hand;

            // 添加鼠标悬停效果
            btn.MouseEnter += (s, e) =>
            {
                var button = s as Button;
                if (button != null)
                {
                    button.BackColor = AdjustBrightness(backColor, 1.1);
                }
            };
            btn.MouseLeave += (s, e) =>
            {
                var button = s as Button;
                if (button != null)
                {
                    button.BackColor = backColor;
                }
            };
        }

        /// <summary>
        /// 设置卡片按钮样式（大按钮，用于主页面）
        /// </summary>
        public static void StyleCardButton(Button btn, Color backColor, Color foreColor = default)
        {
            if (foreColor == default)
                foreColor = Colors.LightText;

            btn.FlatStyle = FlatStyle.Flat;
            btn.FlatAppearance.BorderSize = 0;
            btn.BackColor = backColor;
            btn.ForeColor = foreColor;
            btn.Font = new Font("微软雅黑", 13, FontStyle.Bold);
            btn.Cursor = Cursors.Hand;
            btn.TextAlign = ContentAlignment.MiddleCenter;

            // 添加鼠标悬停效果
            btn.MouseEnter += (s, e) =>
            {
                var button = s as Button;
                if (button != null)
                {
                    button.BackColor = AdjustBrightness(backColor, 1.15);
                    button.Font = new Font("微软雅黑", 14, FontStyle.Bold);
                }
            };
            btn.MouseLeave += (s, e) =>
            {
                var button = s as Button;
                if (button != null)
                {
                    button.BackColor = backColor;
                    button.Font = new Font("微软雅黑", 13, FontStyle.Bold);
                }
            };

            // 点击反馈
            btn.MouseDown += (s, e) =>
            {
                var button = s as Button;
                if (button != null)
                {
                    button.BackColor = AdjustBrightness(backColor, 0.85);
                }
            };
            btn.MouseUp += (s, e) =>
            {
                var button = s as Button;
                if (button != null)
                {
                    button.BackColor = AdjustBrightness(backColor, 1.15);
                }
            };
        }

        /// <summary>
        /// 设置表单风格
        /// </summary>
        public static void StyleForm(Form form)
        {
            form.BackColor = Colors.Background;
            form.Font = new Font("微软雅黑", 10);
            form.StartPosition = FormStartPosition.CenterScreen;
        }

        /// <summary>
        /// 美化 DataGridView
        /// </summary>
        public static void StyleDataGridView(DataGridView dgv)
        {
            // 表头样式
            dgv.ColumnHeadersDefaultCellStyle.BackColor = Colors.Primary;
            dgv.ColumnHeadersDefaultCellStyle.ForeColor = Colors.LightText;
            dgv.ColumnHeadersDefaultCellStyle.Font = new Font("微软雅黑", 11, FontStyle.Bold);
            dgv.ColumnHeadersDefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleCenter;
            dgv.ColumnHeadersHeight = 35;

            // 行样式
            dgv.DefaultCellStyle.Font = new Font("微软雅黑", 10);
            dgv.DefaultCellStyle.ForeColor = Colors.DarkText;
            dgv.DefaultCellStyle.SelectionBackColor = Colors.Primary;
            dgv.DefaultCellStyle.SelectionForeColor = Colors.LightText;
            dgv.RowTemplate.Height = 30;

            // 交替行颜色
            dgv.AlternatingRowsDefaultCellStyle.BackColor = Color.FromArgb(245, 245, 245);

            // 网格线颜色
            dgv.GridColor = Colors.BorderColor;
            dgv.BorderStyle = BorderStyle.Fixed3D;

            // 尝试通过反射启用双缓冲
            try
            {
                var property = typeof(DataGridView).GetProperty("DoubleBuffered", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
                if (property != null && property.CanWrite)
                {
                    property.SetValue(dgv, true);
                }
            }
            catch { }
        }

        /// <summary>
        /// 设置标签样式
        /// </summary>
        public static void StyleLabel(Label label, bool isBold = false)
        {
            label.ForeColor = Colors.DarkText;
            label.Font = isBold ? new Font("微软雅黑", 11, FontStyle.Bold) : new Font("微软雅黑", 10);
        }

        /// <summary>
        /// 设置文本框样式
        /// </summary>
        public static void StyleTextBox(TextBox textBox)
        {
            textBox.BorderStyle = BorderStyle.FixedSingle;
            textBox.Font = new Font("微软雅黑", 10);
            textBox.ForeColor = Colors.DarkText;
            textBox.Height = 30;
        }

        /// <summary>
        /// 设置组合框样式
        /// </summary>
        public static void StyleComboBox(ComboBox comboBox)
        {
            comboBox.Font = new Font("微软雅黑", 10);
            comboBox.ForeColor = Colors.DarkText;
            comboBox.Height = 30;
        }

        /// <summary>
        /// 调整颜色亮度
        /// </summary>
        private static Color AdjustBrightness(Color color, double factor)
        {
            return Color.FromArgb(
                (int)(color.R * factor > 255 ? 255 : color.R * factor),
                (int)(color.G * factor > 255 ? 255 : color.G * factor),
                (int)(color.B * factor > 255 ? 255 : color.B * factor));
        }

        /// <summary>
        /// 创建圆角的 GraphicsPath（用于自定义绘制）
        /// </summary>
        public static GraphicsPath GetRoundedPath(Rectangle rect, int radius)
        {
            var path = new GraphicsPath();
            int diameter = radius * 2;
            var arc = new Rectangle(rect.Location, new Size(diameter, diameter));

            path.AddArc(arc, 180, 90);
            arc.X = rect.Right - diameter;
            path.AddArc(arc, 270, 90);
            arc.Y = rect.Bottom - diameter;
            path.AddArc(arc, 0, 90);
            arc.X = rect.Left;
            path.AddArc(arc, 90, 90);
            path.CloseFigure();

            return path;
        }
    }
}

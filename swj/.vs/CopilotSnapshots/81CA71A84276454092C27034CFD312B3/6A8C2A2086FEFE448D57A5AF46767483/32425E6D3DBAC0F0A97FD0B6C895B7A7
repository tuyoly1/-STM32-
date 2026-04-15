using System;
using System.Windows.Forms;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace WinFormsApp1
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
            this.DoubleBuffered = true;
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            UIHelper.StyleForm(this);
            StyleButtons();
            
            // 设置头部背景为静态蓝色
            pnlHeader.BackColor = UIHelper.Colors.Primary;
            
            // 应用圆角效果
            ApplyRoundCorners();
            
            // 使用后台线程加载虚化背景，避免卡死
            System.Threading.Thread bgThread = new System.Threading.Thread(() =>
            {
                SetBlurredBackground();
            })
            {
                IsBackground = true
            };
            bgThread.Start();
        }

        private void ApplyRoundCorners()
        {
            // 设置窗体圆角
            int cornerRadius = 20;
            SetRoundCorners(this, cornerRadius);
            
            // 设置头部面板圆角
            SetRoundCorners(pnlHeader, 20);
            
            // 设置按钮圆角
            SetRoundCorners(button1, 12);
            SetRoundCorners(button2, 12);
            SetRoundCorners(button3, 12);
            SetRoundCorners(button4, 12);
            SetRoundCorners(button5, 12);
        }

        private void SetRoundCorners(Control control, int radius)
        {
            GraphicsPath path = new GraphicsPath();
            
            Rectangle rect = new Rectangle(0, 0, control.Width, control.Height);
            int diameter = radius * 2;
            
            // 四个圆角
            path.AddArc(rect.X, rect.Y, diameter, diameter, 180, 90);
            path.AddArc(rect.X + rect.Width - diameter, rect.Y, diameter, diameter, 270, 90);
            path.AddArc(rect.X + rect.Width - diameter, rect.Y + rect.Height - diameter, diameter, diameter, 0, 90);
            path.AddArc(rect.X, rect.Y + rect.Height - diameter, diameter, diameter, 90, 90);
            path.CloseFigure();
            
            control.Region = new Region(path);
            path.Dispose();
        }

        private void SetBlurredBackground()
        {
            string imagePath = "tsg.png";
            
            try
            {
                if (System.IO.File.Exists(imagePath))
                {
                    // 加载原始图片
                    using (Image originalImage = Image.FromFile(imagePath))
                    {
                        // 创建虚化后的图片
                        Bitmap blurredImage = ApplyBlurEffect(originalImage, this.ClientSize);
                        
                        // 在 UI 线程中设置背景
                        this.Invoke(new Action(() =>
                        {
                            this.BackgroundImage = blurredImage;
                            this.BackgroundImageLayout = ImageLayout.Stretch;
                            System.Diagnostics.Debug.WriteLine($"✓ 成功加载和虚化背景图片");
                        }));
                    }
                }
                else
                {
                    System.Diagnostics.Debug.WriteLine($"✗ 找不到图片文件: {imagePath}");
                    
                    // 尝试其他可能的路径
                    string[] possiblePaths = new[]
                    {
                        "tsg.png",
                        System.IO.Path.Combine("bin", "Debug", "net8.0-windows", "tsg.png"),
                        System.IO.Path.Combine("..", "bin", "Debug", "net8.0-windows", "tsg.png"),
                        System.IO.Path.Combine("Resources", "tsg.png"),
                    };
                    
                    bool found = false;
                    foreach (var path in possiblePaths)
                    {
                        if (System.IO.File.Exists(path))
                        {
                            using (Image originalImage = Image.FromFile(path))
                            {
                                Bitmap blurredImage = ApplyBlurEffect(originalImage, this.ClientSize);
                                this.Invoke(new Action(() =>
                                {
                                    this.BackgroundImage = blurredImage;
                                    this.BackgroundImageLayout = ImageLayout.Stretch;
                                    System.Diagnostics.Debug.WriteLine($"✓ 在 {path} 找到并加载虚化背景图片");
                                }));
                                found = true;
                                break;
                            }
                        }
                    }
                    
                    if (!found)
                    {
                        System.Diagnostics.Debug.WriteLine($"✗ 无法找到图片，使用默认灰色背景");
                        this.Invoke(new Action(() =>
                        {
                            this.BackColor = Color.FromArgb(240, 240, 240);
                        }));
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"✗ 加载背景图片出错: {ex.Message}");
                this.Invoke(new Action(() =>
                {
                    this.BackColor = Color.FromArgb(240, 240, 240);
                }));
            }
        }

        private Bitmap ApplyBlurEffect(Image sourceImage, Size targetSize)
        {
            // 创建目标大小的位图
            Bitmap resizedImage = new Bitmap(targetSize.Width, targetSize.Height);
            using (Graphics g = Graphics.FromImage(resizedImage))
            {
                g.InterpolationMode = InterpolationMode.HighQualityBicubic;
                g.DrawImage(sourceImage, 0, 0, targetSize.Width, targetSize.Height);
            }
            
            // 使用快速虚化方法：先缩小再放大（更高效）
            int blurScale = 8; // 虚化倍数
            int smallWidth = Math.Max(1, targetSize.Width / blurScale);
            int smallHeight = Math.Max(1, targetSize.Height / blurScale);
            
            // 缩小图片
            Bitmap smallImage = new Bitmap(smallWidth, smallHeight);
            using (Graphics g = Graphics.FromImage(smallImage))
            {
                g.InterpolationMode = InterpolationMode.HighQualityBicubic;
                g.DrawImage(resizedImage, 0, 0, smallWidth, smallHeight);
            }
            
            // 放大回原大小（产生虚化效果）
            Bitmap blurredBitmap = new Bitmap(targetSize.Width, targetSize.Height);
            using (Graphics g = Graphics.FromImage(blurredBitmap))
            {
                g.InterpolationMode = InterpolationMode.HighQualityBicubic;
                g.DrawImage(smallImage, 0, 0, targetSize.Width, targetSize.Height);
            }
            
            resizedImage.Dispose();
            smallImage.Dispose();
            
            return blurredBitmap;
        }

        private void StyleButtons()
        {
            var buttonStyles = new[]
            {
                (button: button1, color: UIHelper.Colors.Primary),
                (button: button2, color: UIHelper.Colors.Secondary),
                (button: button3, color: UIHelper.Colors.Warning),
                (button: button4, color: UIHelper.Colors.Info),
                (button: button5, color: UIHelper.Colors.Danger)
            };

            foreach (var (button, color) in buttonStyles)
            {
                StyleCardButton(button, color);
            }
        }

        private void StyleCardButton(Button btn, Color bgColor)
        {
            btn.FlatStyle = FlatStyle.Flat;
            btn.FlatAppearance.BorderSize = 0;
            btn.BackColor = bgColor;
            btn.ForeColor = UIHelper.Colors.LightText;
            btn.Font = new Font("微软雅黑", 12, FontStyle.Bold);
            btn.Cursor = Cursors.Hand;
            btn.TextAlign = ContentAlignment.MiddleCenter;
            
            btn.MouseEnter += (s, e) =>
            {
                var button = s as Button;
                if (button != null)
                {
                    button.BackColor = AdjustBrightness(bgColor, 1.15);
                }
            };

            btn.MouseLeave += (s, e) =>
            {
                var button = s as Button;
                if (button != null)
                {
                    button.BackColor = bgColor;
                }
            };

            btn.MouseDown += (s, e) =>
            {
                var button = s as Button;
                if (button != null)
                {
                    button.BackColor = AdjustBrightness(bgColor, 0.9);
                }
            };

            btn.MouseUp += (s, e) =>
            {
                var button = s as Button;
                if (button != null)
                {
                    button.BackColor = AdjustBrightness(bgColor, 1.15);
                }
            };
        }

        private Color AdjustBrightness(Color color, double factor)
        {
            return Color.FromArgb(
                (int)(color.R * factor > 255 ? 255 : color.R * factor),
                (int)(color.G * factor > 255 ? 255 : color.G * factor),
                (int)(color.B * factor > 255 ? 255 : color.B * factor));
        }

        private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            Application.Exit();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            UserManageForm userForm = new UserManageForm();
            userForm.ShowDialog();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            OperationLogForm logForm = new OperationLogForm();
            logForm.ShowDialog();
        }

       
        private void button5_Click(object sender, EventArgs e)
        {
            CredentialManageForm credentialForm = new CredentialManageForm();
            credentialForm.ShowDialog();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            AccessRecordForm recordForm = new AccessRecordForm();
            recordForm.ShowDialog();
        }

        private void button4_Click(object sender, EventArgs e)
        {
            SerialPortForm serialForm = new SerialPortForm();
            serialForm.ShowDialog();
        }
    }
}
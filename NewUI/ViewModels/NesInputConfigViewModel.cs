﻿#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia;
using Avalonia.Controls;
using Mesen.Config;
using Mesen.Config.Shortcuts;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.Views;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class NesInputConfigViewModel : ViewModelBase
	{
		[Reactive] public NesConfig Config { get; set; }
		
		public ReactiveCommand<Button, Unit> SetupPlayer1 { get; }
		public ReactiveCommand<Button, Unit> SetupPlayer2 { get; }
		public ReactiveCommand<Button, Unit> SetupPlayer3 { get; }
		public ReactiveCommand<Button, Unit> SetupPlayer4 { get; }
		public ReactiveCommand<Button, Unit> SetupPlayer5 { get; }
		public List<ShortcutKeyInfo> ShortcutKeys { get; set; }

		[ObservableAsProperty] public bool HasFourPlayerAdapter { get; }

		public Enum[] AvailableControllerTypesP12 => new Enum[] {
			ControllerType.None,
			ControllerType.NesController,
			ControllerType.FamicomController,
			ControllerType.NesZapper,
			ControllerType.NesArkanoidController,
			ControllerType.PowerPad,
			ControllerType.SnesController,
			ControllerType.SnesMouse,
			ControllerType.SuborMouse,
			ControllerType.VbController
		};

		public Enum[] AvailableControllerTypesP34 => new Enum[] {
			ControllerType.None,
			ControllerType.NesController,
			ControllerType.FamicomController,
			ControllerType.SnesController
		};

		public Enum[] AvailableExpansionTypes => new Enum[] {
			ControllerType.None,
			ControllerType.FourScore,
			ControllerType.FamicomZapper,
			ControllerType.FourPlayerAdapter,
			ControllerType.FamicomArkanoidController,
			ControllerType.OekaKidsTablet,
			ControllerType.FamilyTrainerMat,
			ControllerType.KonamiHyperShot,
			ControllerType.FamilyBasicKeyboard,
			ControllerType.PartyTap,
			ControllerType.Pachinko,
			ControllerType.ExcitingBoxing,
			ControllerType.JissenMahjong,
			ControllerType.SuborKeyboard,
			ControllerType.BarcodeBattler,
			ControllerType.HoriTrack,
			ControllerType.BandaiHyperShot,
			ControllerType.AsciiTurboFile,
			ControllerType.BattleBox
		};

		//For designer preview
		public NesInputConfigViewModel() : this(new NesConfig(), new PreferencesConfig()) { }

		public NesInputConfigViewModel(NesConfig config, PreferencesConfig preferences)
		{
			Config = config;

			this.WhenAnyValue(x => x.Config.Controllers[4].Type).Select(t => t == ControllerType.FourPlayerAdapter || t == ControllerType.FourScore).ToPropertyEx(this, x => x.HasFourPlayerAdapter);

			IObservable<bool> button1Enabled = this.WhenAnyValue(x => x.Config.Controllers[0].Type, x => x.CanConfigure());
			this.SetupPlayer1 = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 0), button1Enabled);

			IObservable<bool> button2Enabled = this.WhenAnyValue(x => x.Config.Controllers[1].Type, x => x.CanConfigure());
			this.SetupPlayer2 = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 1), button2Enabled);

			IObservable<bool> button3Enabled = this.WhenAnyValue(x => x.Config.Controllers[2].Type, x => x.CanConfigure());
			this.SetupPlayer3 = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 2), button3Enabled);

			IObservable<bool> button4Enabled = this.WhenAnyValue(x => x.Config.Controllers[3].Type, x => x.CanConfigure());
			this.SetupPlayer4 = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 3), button4Enabled);

			IObservable<bool> button5Enabled = this.WhenAnyValue(x => x.Config.Controllers[4].Type, x => x.CanConfigure());
			this.SetupPlayer5 = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 4), button5Enabled);

			EmulatorShortcut[] displayOrder = new EmulatorShortcut[] {
				EmulatorShortcut.FdsSwitchDiskSide,
				EmulatorShortcut.FdsEjectDisk,
				EmulatorShortcut.FdsInsertNextDisk,
				EmulatorShortcut.VsInsertCoin1,
				EmulatorShortcut.VsInsertCoin2,
				EmulatorShortcut.VsInsertCoin3,
				EmulatorShortcut.VsInsertCoin4,
				EmulatorShortcut.VsServiceButton,
				EmulatorShortcut.VsServiceButton2,
				EmulatorShortcut.NesInputBarcode
			};

			Dictionary<EmulatorShortcut, ShortcutKeyInfo> shortcuts = new Dictionary<EmulatorShortcut, ShortcutKeyInfo>();
			foreach(ShortcutKeyInfo shortcut in preferences.ShortcutKeys) {
				shortcuts[shortcut.Shortcut] = shortcut;
			}

			if(Design.IsDesignMode) {
				return;
			}

			ShortcutKeys = new List<ShortcutKeyInfo>();
			for(int i = 0; i < displayOrder.Length; i++) {
				ShortcutKeys.Add(shortcuts[displayOrder[i]]);
			}
		}

		private async void OpenSetup(Button btn, int port)
		{
			PixelPoint startPosition = btn.PointToScreen(new Point(-7, btn.Height));
			ControllerConfigWindow wnd = new ControllerConfigWindow();
			wnd.WindowStartupLocation = WindowStartupLocation.Manual;
			wnd.Position = startPosition;

			NesControllerConfig cfg = JsonHelper.Clone(this.Config.Controllers[port]);
			wnd.DataContext = new ControllerConfigViewModel(cfg);

			if(await wnd.ShowDialog<bool>(btn.Parent?.VisualRoot as Window)) {
				//Create a new list to trigger UI refresh
				List<NesControllerConfig> controllers = new List<NesControllerConfig>(Config.Controllers);
				controllers[port] = cfg;
				Config.Controllers = controllers;
			}
		}
	}
}

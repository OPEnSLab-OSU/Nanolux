import { h } from 'preact';
import { Link } from 'preact-router/match';
import style from './style.css';
import PaletteSelector from '../palette_selector';


/**
 * Generates the header object for the website. This provides links
 * to the settings and wifi pages. It also loads the CymaSpace logo
 * for display.
 * 
 * @return The HTML object for the header.
 */
const Header = () => (
	<header className={style.header}>
		<a href="/" className={style.logo}>
			<img src="../../assets/cymalog1.png" alt="CymaSpace Logo" height="32" />
			<h1>AudioLux</h1>
		</a>
		<div className={style.palette}>
			<PaletteSelector />
		</div>
		<nav>
			<Link activeClassName={style.active} href="/">
				Settings
			</Link>
			<Link activeClassName={style.active} href="/wifi">
				Wifi
			</Link>
		</nav>
	</header>
);

export default Header;

import { Tooltip } from 'react-tooltip';
import 'react-tooltip/dist/react-tooltip.css';

/**
 * @brief A span that, when hovered or focused, displays a tooltip with the provided content.
 *
 * Renders a clickable/focusable span that, when hovered or focused, displays a tooltip with the provided content.
 *
 * @param id        The unique identifier for this tooltip; links the span and React Tooltip component.
 * @param content   The ReactNode content to render inside the tooltip popup.
 * @param offset    (Optional) The pixel offset between the trigger and the tooltip (default: 10).
 * @param style     (Optional) CSS styles applied to the trigger span. Defaults to `{ fontSize: '1.2rem', cursor: 'pointer' }`.
 * @param label     (Optional) The ReactNode to render inside the trigger span. Defaults to `'(?)'`.
 * @param ...props  Any additional props to spread onto the trigger span (e.g., event handlers, aria attributes).
 *
 * @returns The TooltipWrapper UI element.
 */
const TooltipWrapper = ({
    id,
    content,
    offset = 10,
    style = { fontSize: '1.2rem', cursor: 'pointer' },
    label = '(?)',
    ...props
}) => (
    <>
        <span
          style={style}
          data-tooltip-id={id}
          data-tooltip-offset={offset}
          role='button'
          tabIndex={0}
          aria-describedby={id}
          {...props}
        >
            {label}
        </span>
        <Tooltip
          id={id}
          content={content}
          style={style}
          aria-hidden='true'
        />
    </>
);

export default TooltipWrapper;